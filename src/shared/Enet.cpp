#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#undef near
#undef far


#include <winsock2.h>
#include <windows.h>
#define ENET_IMPLEMENTATION
#include <enet.h>
#include "Enet.hpp"
#include <iostream>

namespace Net {
    struct Wrapper::Impl {
        ENetHost *host = nullptr;
        ENetPeer *peer = nullptr;
    };

    Wrapper::Wrapper() : pImpl(new Impl{}) {
    }

    Wrapper::~Wrapper() {
        disconnect();
        enet_deinitialize();
#ifdef _WIN32
        WSACleanup();
#endif
        delete pImpl;
    }

    bool Wrapper::init() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed: " << WSAGetLastError() << "\n";
            return false;
        }
#endif
        if (enet_initialize() != 0) {
            std::cerr << "Failed to initialize ENet\n";
#ifdef _WIN32
            WSACleanup();
#endif
            return false;
        }
        return true;
    }

    bool Wrapper::listen(uint16_t port) {
        if (pImpl->host) {
            std::cerr << "Host already created\n";
            return false;
        }
        ENetAddress addr{};
        addr.host = ENET_HOST_ANY;
        addr.port = port;

        pImpl->host = enet_host_create(&addr, 32, 2, 0, 0);
        if (!pImpl->host) {
#ifdef _WIN32
            int wsaErr = WSAGetLastError();
            DWORD errWin = GetLastError();
            std::cerr << "Bind failed on ENET_HOST_ANY port=" << port
                    << "  WSA error=" << wsaErr
                    << "  GetLastError=" << errWin << "\n";
#else
            std::cerr << "Bind failed on 0.0.0.0 port=" << port
                      << "  errno=" << errno << "\n";
#endif
            return false;
        }

        std::cout << "Listening on port " << port << "\n";
        return true;
    }

    bool Wrapper::connect(const char *host, uint16_t port) {
        ENetAddress addr;
        if (enet_address_set_host(&addr, host) != 0) {
            std::cerr << "Failed to resolve host: " << host << "\n";
            return false;
        }
        addr.port = port;
        pImpl->host = enet_host_create(nullptr, 1, 2, 0, 0);
        if (!pImpl->host) {
            std::cerr << "Failed to create ENet client host\n";
            return false;
        }
        pImpl->peer = enet_host_connect(pImpl->host, &addr, 2, 0);
        if (!pImpl->peer) {
            std::cerr << "enet_host_connect to " << host << ":" << port << " failed\n";
            enet_host_destroy(pImpl->host);
            pImpl->host = nullptr;
            return false;
        }
        enet_host_flush(pImpl->host);

        // wait up to 5s for the connect event
        ENetEvent evt;
        if (enet_host_service(pImpl->host, &evt, 5000) <= 0 ||
            evt.type != ENET_EVENT_TYPE_CONNECT) {
            std::cerr << "Wrapper::connect: handshake timed out\n";
            disconnect();
            return false;
        }

        return true;
    }

    void Wrapper::service(uint32_t timeoutMs, std::function<void(const Event &)> callback) {
        if (!pImpl->host) return;
        ENetEvent evt;
        while (enet_host_service(pImpl->host, &evt, timeoutMs) > 0) {
            Event e{};
            e.peer = evt.peer;
            switch (evt.type) {
                case ENET_EVENT_TYPE_CONNECT: e.type = EventType::Connect;
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    e.type = EventType::Receive;
                    e.data = evt.packet->data;
                    e.dataLength = evt.packet->dataLength;
                    enet_packet_destroy(evt.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    e.type = EventType::Disconnect;
                    break;
                default: continue;
            }
            callback(e);
        }
    }

    void Wrapper::disconnect() {
        if (pImpl->peer) {
            enet_peer_disconnect(pImpl->peer, 0);
            ENetEvent evt;
            while (enet_host_service(pImpl->host, &evt, 1000) > 0) {
                if (evt.type == ENET_EVENT_TYPE_RECEIVE) enet_packet_destroy(evt.packet);
                else if (evt.type == ENET_EVENT_TYPE_DISCONNECT) break;
            }
            pImpl->peer = nullptr;
        }
        if (pImpl->host) {
            enet_host_destroy(pImpl->host);
            pImpl->host = nullptr;
        }
    }

    void Wrapper::send(const uint8_t *data, size_t len) {
        if (!pImpl->host) return;
        ENetPacket *packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
        if (pImpl->peer) enet_peer_send(pImpl->peer, 0, packet);
        else enet_host_broadcast(pImpl->host, 0, packet);
        enet_host_flush(pImpl->host);
    }

    void Wrapper::send(ENetPeer *peer, const uint8_t *data, size_t len) {
        if (!pImpl->host || !peer) return;
        ENetPacket *packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(pImpl->host);
    }
}
