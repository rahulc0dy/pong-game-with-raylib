// include/game/NetworkManager.hpp
#pragma once

#include "Enet.hpp"
#include "Ball.hpp"
#include "Paddle.hpp"
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>

enum class NetworkState {
    Disconnected,
    Connecting,
    HostingWaiting,
    JoinedWaiting,
    Connected
};

struct GamePacket {
    float ballX, ballY;
    float ballSpeedX, ballSpeedY;
    float player1Y, player2Y;
    int playerScore, cpuScore;
};

class NetworkManager {
public:
    NetworkManager();

    ~NetworkManager();

    bool init();

    bool hostGame(std::string &outRoomCode);

    bool joinGame(const std::string &roomCode);

    void disconnect();

    void update(); // Call from main thread

    void sendGameState(const Ball &ball, const Paddle &player1, const Paddle &player2);

    void sendPaddleInput(const Paddle &paddle);

    bool getGameState(Ball &ball, Paddle &player1, Paddle &player2);

    bool getPaddleInput(Paddle &paddle);

    bool isConnected() const { return m_state == NetworkState::Connected; }
    bool isHost() const { return m_isHost; }
    NetworkState getState() const { return m_state; }
    std::string getRoomCode() const { return m_roomCode; }

private:
    void networkThread();

    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<NetworkState> m_state;

    std::mutex m_sendMutex;
    std::mutex m_receiveMutex;
    std::queue<GamePacket> m_gameStateQueue;
    std::queue<float> m_paddleInputQueue;

    Net::Wrapper m_net;
    std::string m_roomCode;
    bool m_isHost;

    // For thread synchronization
    std::mutex m_stateMutex;
    std::condition_variable m_stateCV;
 };