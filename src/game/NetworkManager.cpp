#include "NetworkManager.hpp"
#include <chrono>
#include <iostream>

enum class MsgType : uint8_t {
    CreateRoom = 1,
    JoinRoom = 2,
    RoomCreated = 3,
    RoomJoined = 4,
    Error = 5,
    GameState = 6,
    PaddleInput = 7
};

NetworkManager::NetworkManager()
    : m_running(false), m_state(NetworkState::Disconnected), m_isHost(false) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::init() {
    return m_net.init();
}

bool NetworkManager::hostGame(std::string &outRoomCode) {
    if (m_state != NetworkState::Disconnected)
        return false;

    if (!m_net.connect("127.0.0.1", PORT_NUMBER))
        return false;

    m_state = NetworkState::Connecting;
    m_isHost = true;

    // Start network thread
    m_running = true;
    m_thread = std::thread(&NetworkManager::networkThread, this);

    // Wait for room creation
    std::unique_lock<std::mutex> lock(m_stateMutex);
    if (!m_stateCV.wait_for(lock, std::chrono::seconds(5),
                            [this] { return m_state == NetworkState::HostingWaiting; })) {
        disconnect();
        return false;
    }

    outRoomCode = m_roomCode;
    return true;
}

bool NetworkManager::joinGame(const std::string &roomCode) {
    if (m_state != NetworkState::Disconnected)
        return false;

    if (!m_net.connect("127.0.0.1", PORT_NUMBER))
        return false;

    m_state = NetworkState::Connecting;
    m_isHost = false;
    m_roomCode = roomCode;

    // Start network thread
    m_running = true;
    m_thread = std::thread(&NetworkManager::networkThread, this);

    // Wait for room join
    std::unique_lock<std::mutex> lock(m_stateMutex);
    if (!m_stateCV.wait_for(lock, std::chrono::seconds(5),
                            [this] {
                                return m_state == NetworkState::JoinedWaiting
                                       || m_state == NetworkState::Connected;
                            })) {
        disconnect();
        return false;
    }

    return true;
}

void NetworkManager::disconnect() {
    m_running = false;

    if (m_thread.joinable())
        m_thread.join();

    m_net.disconnect();
    m_state = NetworkState::Disconnected;
}

void NetworkManager::update() {
    // Handle state transitions from background thread
    if (m_state == NetworkState::HostingWaiting || m_state == NetworkState::JoinedWaiting) {
        // Check if connected based on received packets
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (!m_gameStateQueue.empty() || !m_paddleInputQueue.empty()) {
            m_state = NetworkState::Connected;
            m_stateCV.notify_all();
        }
    }
}

void NetworkManager::sendGameState(const Ball &ball, const Paddle &player1, const Paddle &player2) {
    if (!isHost() || m_state != NetworkState::Connected)
        return;

    GamePacket packet{
        ball.x, ball.y,
        ball.speed_x, ball.speed_y,
        player1.y, player2.y,
        ball.playerScore, ball.cpuScore
    };

    uint8_t buffer[sizeof(GamePacket) + 1];
    buffer[0] = static_cast<uint8_t>(MsgType::GameState);
    memcpy(buffer + 1, &packet, sizeof(GamePacket));

    m_net.send(buffer, sizeof(buffer));
}

void NetworkManager::sendPaddleInput(const Paddle &paddle) {
    if (isHost() || m_state != NetworkState::Connected)
        return;

    float y = paddle.y;
    uint8_t buffer[sizeof(float) + 1];
    buffer[0] = static_cast<uint8_t>(MsgType::PaddleInput);
    memcpy(buffer + 1, &y, sizeof(float));

    m_net.send(buffer, sizeof(buffer));
}

bool NetworkManager::getGameState(Ball &ball, Paddle &player1, Paddle &player2) {
    if (isHost() || m_state != NetworkState::Connected)
        return false;

    std::lock_guard<std::mutex> lock(m_receiveMutex);
    if (m_gameStateQueue.empty())
        return false;

    GamePacket packet = m_gameStateQueue.front();
    m_gameStateQueue.pop();

    ball.x = packet.ballX;
    ball.y = packet.ballY;
    ball.speed_x = packet.ballSpeedX;
    ball.speed_y = packet.ballSpeedY;
    player1.y = packet.player1Y;
    player2.y = packet.player2Y;
    ball.playerScore = packet.playerScore;
    ball.cpuScore = packet.cpuScore;

    return true;
}

bool NetworkManager::getPaddleInput(Paddle &paddle) {
    if (!isHost() || m_state != NetworkState::Connected)
        return false;

    std::lock_guard<std::mutex> lock(m_receiveMutex);
    if (m_paddleInputQueue.empty())
        return false;

    paddle.y = m_paddleInputQueue.front();
    m_paddleInputQueue.pop();

    return true;
}

void NetworkManager::networkThread() {
    // Initial message based on host/client
    if (m_isHost) {
        // Send create room request
        uint8_t createMsg = static_cast<uint8_t>(MsgType::CreateRoom);
        m_net.send(&createMsg, 1);
    } else {
        // Send join room request
        uint8_t joinMsg[6] = {static_cast<uint8_t>(MsgType::JoinRoom)};
        memcpy(joinMsg + 1, m_roomCode.c_str(), 5);
        m_net.send(joinMsg, 6);
    }

    while (m_running) {
        m_net.service(10, [this](const Net::Event &evt) {
            if (evt.type == Net::EventType::Receive) {
                if (evt.dataLength < 1) return;

                MsgType type = static_cast<MsgType>(evt.data[0]);
                switch (type) {
                    case MsgType::RoomCreated: {
                        if (evt.dataLength >= 6) {
                            char code[6] = {};
                            memcpy(code, evt.data + 1, 5);
                            m_roomCode = std::string(code);

                            std::lock_guard<std::mutex> lock(m_stateMutex);
                            m_state = NetworkState::HostingWaiting;
                            m_stateCV.notify_all();
                        }
                        break;
                    }
                    case MsgType::RoomJoined: {
                        std::lock_guard<std::mutex> lock(m_stateMutex);
                        m_state = NetworkState::JoinedWaiting;
                        m_stateCV.notify_all();
                        break;
                    }
                    case MsgType::GameState: {
                        if (evt.dataLength == sizeof(GamePacket) + 1) {
                            std::lock_guard<std::mutex> lock(m_receiveMutex);
                            GamePacket packet;
                            memcpy(&packet, evt.data + 1, sizeof(GamePacket));
                            m_gameStateQueue.push(packet);
                        }
                        break;
                    }
                    case MsgType::PaddleInput: {
                        if (evt.dataLength == sizeof(float) + 1) {
                            std::lock_guard<std::mutex> lock(m_receiveMutex);
                            float y;
                            memcpy(&y, evt.data + 1, sizeof(float));
                            m_paddleInputQueue.push(y);
                        }
                        break;
                    }
                    case MsgType::Error: {
                        std::lock_guard<std::mutex> lock(m_stateMutex);
                        m_state = NetworkState::Disconnected;
                        m_stateCV.notify_all();
                        break;
                    }
                    default:
                        break;
                }
            } else if (evt.type == Net::EventType::Disconnect) {
                std::lock_guard<std::mutex> lock(m_stateMutex);
                m_state = NetworkState::Disconnected;
                m_stateCV.notify_all();
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
