#include "Enet.hpp"
#include <unordered_map>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <signal.h>

#include "enet.h"

Net::Wrapper *g_net = nullptr;
std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    std::cout << "\nShutting down server..." << std::endl;
    g_running = false;
}

enum class MsgType : uint8_t {
    CreateRoom = 1,
    JoinRoom = 2,
    RoomCreated = 3,
    RoomJoined = 4,
    Error = 5,
    GameState = 6,
    PaddleInput = 7
};

struct ClientInfo {
    int id;
    std::string roomCode;
    ENetPeer *peer; // Store the peer pointer
};

struct Room {
    std::string code;
    int player1Id = -1;
    int player2Id = -1;
    bool isFull() const { return player1Id != -1 && player2Id != -1; }
    bool isEmpty() const { return player1Id == -1 && player2Id == -1; }

    int getOtherPlayer(int playerId) const {
        return playerId == player1Id ? player2Id : player1Id;
    }
};

std::string generateRoomCode() {
    static const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::string code;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    for (int i = 0; i < 5; ++i) code += chars[dis(gen)];
    return code;
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Net::Wrapper net;
    g_net = &net;

    if (!net.init()) {
        std::cerr << "Failed to initialize ENet\n";
        return 1;
    }

    if (!net.listen(PORT_NUMBER)) {
        std::cerr << "Failed to listen on port " << PORT_NUMBER << " (port might be in use)\n";
        return 1;
    }

    std::unordered_map<std::string, Room> rooms;
    std::unordered_map<int, ClientInfo> clients;
    std::unordered_map<ENetPeer *, int> peerToClientId; // Use ENetPeer* directly
    int nextClientId = 1;

    std::cout << "Server started on port " << PORT_NUMBER << std::endl;

    while (g_running) {
        net.service(10, [&](const Net::Event &evt) {
            if (evt.type == Net::EventType::Connect) {
                int clientId = nextClientId++;
                clients[clientId] = ClientInfo{clientId, "", evt.peer};
                peerToClientId[evt.peer] = clientId;
                std::cout << "Client connected: " << clientId << "\n";
            } else if (evt.type == Net::EventType::Receive) {
                if (evt.dataLength < 1) return;
                int clientId = peerToClientId[evt.peer];
                MsgType type = static_cast<MsgType>(evt.data[0]);
                switch (type) {
                    case MsgType::CreateRoom: {
                        std::string code = generateRoomCode();
                        while (rooms.find(code) != rooms.end()) {
                            code = generateRoomCode();
                        }
                        Room room;
                        room.code = code;
                        room.player1Id = clientId;
                        rooms[code] = room;
                        clients[clientId].roomCode = code;
                        // Send room code back
                        uint8_t msg[6] = {static_cast<uint8_t>(MsgType::RoomCreated)};
                        std::memcpy(msg + 1, code.c_str(), 5);
                        net.send(evt.peer, msg, 6);
                        std::cout << "Room created: " << code << " by client " << clientId << "\n";
                        break;
                    }
                    case MsgType::JoinRoom: {
                        if (evt.dataLength < 6) break;
                        char code[6] = {};
                        std::memcpy(code, evt.data + 1, 5);
                        std::string roomCode(code);
                        auto it = rooms.find(roomCode);
                        if (it != rooms.end() && !it->second.isFull()) {
                            it->second.player2Id = clientId;
                            clients[clientId].roomCode = roomCode;
                            // Notify both players that game can start
                            uint8_t msg[6] = {static_cast<uint8_t>(MsgType::RoomJoined)};
                            std::memcpy(msg + 1, code, 5);
                            // Send to joining player
                            net.send(evt.peer, msg, 6);
                            // Find and send to host player
                            auto &host = clients[it->second.player1Id];
                            net.send(host.peer, msg, 6);
                            std::cout << "Client " << clientId << " joined room: " << roomCode << "\n";
                        } else {
                            uint8_t msg[2] = {static_cast<uint8_t>(MsgType::Error), 1};
                            net.send(evt.peer, msg, 2);
                            std::cout << "Client " << clientId << " failed to join room: " << roomCode << "\n";
                        }
                        break;
                    }
                    case MsgType::GameState:
                    case MsgType::PaddleInput: {
                        // Relay to other player in the room
                        auto &client = clients[clientId];
                        if (!client.roomCode.empty()) {
                            auto &room = rooms[client.roomCode];
                            int otherPlayerId = room.getOtherPlayer(clientId);
                            if (otherPlayerId != -1) {
                                // Find peer for other player and send
                                auto &otherClient = clients[otherPlayerId];
                                net.send(otherClient.peer, evt.data, evt.dataLength);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            } else if (evt.type == Net::EventType::Disconnect) {
                auto peerIt = peerToClientId.find(evt.peer);
                if (peerIt != peerToClientId.end()) {
                    int clientId = peerIt->second;
                    auto clientIt = clients.find(clientId);

                    if (clientIt != clients.end()) {
                        // Remove from room
                        std::string roomCode = clientIt->second.roomCode;
                        if (!roomCode.empty()) {
                            auto roomIt = rooms.find(roomCode);
                            if (roomIt != rooms.end()) {
                                Room &room = roomIt->second;
                                if (room.player1Id == clientId) {
                                    room.player1Id = -1;
                                } else if (room.player2Id == clientId) {
                                    room.player2Id = -1;
                                }

                                // Clean up empty room
                                if (room.isEmpty()) {
                                    rooms.erase(roomIt);
                                }
                                // Notify other player about disconnect
                                else {
                                    int otherPlayerId = room.getOtherPlayer(clientId);
                                    if (otherPlayerId != -1) {
                                        auto &otherClient = clients[otherPlayerId];
                                        uint8_t msg[2] = {static_cast<uint8_t>(MsgType::Error), 2};
                                        net.send(otherClient.peer, msg, 2);
                                    }
                                }
                            }
                        }

                        // Remove client
                        clients.erase(clientIt);
                        peerToClientId.erase(peerIt);
                        std::cout << "Client " << clientId << " disconnected\n";
                    }
                }
            }
        });
    }

    net.disconnect();
    enet_deinitialize();
    std::cout << "Server shut down gracefully" << std::endl;
    return 0;
}
