// src/shared/Enet.cpp
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

namespace Net {
    struct Wrapper::Impl {
        ENetHost *host = nullptr;
        ENetPeer *peer = nullptr;
    };

    Wrapper::Wrapper() : pImpl(new Impl{}) {
    }

    Wrapper::~Wrapper() {
        disconnect();
        delete pImpl;
    }

    bool Wrapper::init() {
        return (enet_initialize() == 0);
    }

    bool Wrapper::connect(const char *host, uint16_t port) {
        ENetAddress addr;
        enet_address_set_host(&addr, host);
        addr.port = port;

        pImpl->host = enet_host_create(nullptr, 1, 2, 0, 0);
        if (!pImpl->host) return false;

        pImpl->peer = enet_host_connect(pImpl->host, &addr, 2, 0);
        return pImpl->peer != nullptr;
    }

    void Wrapper::service(unsigned int timeoutMs, std::function<void(const Event &)> callback) {
        if (!pImpl->host) return;

        ENetEvent evt;
        while (enet_host_service(pImpl->host, &evt, timeoutMs) > 0) {
            Event event{};
            event.peer = evt.peer;

            switch (evt.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    event.type = EventType::Connect;
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    event.type = EventType::Receive;
                    event.data = evt.packet->data;
                    event.dataLength = evt.packet->dataLength;
                    enet_packet_destroy(evt.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    event.type = EventType::Disconnect;
                    break;
                default:
                    continue;
            }
            callback(event);
        }
    }

    void Wrapper::disconnect() {
        if (pImpl->peer) {
            enet_peer_disconnect(pImpl->peer, 0);

            // Wait for disconnect acknowledgement
            ENetEvent event;
            while (enet_host_service(pImpl->host, &event, 1000) > 0) {
                if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                    enet_packet_destroy(event.packet);
                } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                    break;
                }
            }

            pImpl->peer = nullptr;
        }

        if (pImpl->host) {
            enet_host_destroy(pImpl->host);
            pImpl->host = nullptr;
        }
    }

    bool Wrapper::listen(uint16_t port) {
        ENetAddress addr;
        addr.host = ENET_HOST_ANY;
        addr.port = port;
        pImpl->host = enet_host_create(&addr, 32, 2, 0, 0); // Support more clients
        return pImpl->host != nullptr;
    }

    void Wrapper::send(const uint8_t *data, size_t len) {
        if (!pImpl->host) return;

        if (pImpl->peer) {
            // Client: send to server
            ENetPacket *packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(pImpl->peer, 0, packet);
        } else {
            // Server: broadcast to all clients
            ENetPacket *packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(pImpl->host, 0, packet);
        }
        enet_host_flush(pImpl->host);
    }

    void Wrapper::send(ENetPeer *peer, const uint8_t *data, size_t len) {
        if (!pImpl->host || !peer) return;

        ENetPacket *packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(pImpl->host);
    }
}
