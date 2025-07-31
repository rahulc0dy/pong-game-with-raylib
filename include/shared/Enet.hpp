// include/Enet.hpp
#pragma once

#include <functional>
#include <cstdint>

// Forward declarations to avoid including enet.h in headers
typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

namespace Net {
    enum class EventType {
        None,
        Connect,
        Disconnect,
        Receive
    };

    struct Event {
        EventType type;
        ENetPeer *peer; // Added peer pointer for server identification
        const uint8_t *data;
        size_t dataLength;
    };

    using EventCallback = std::function<void(const Event &)>;

    class Wrapper {
    public:
        Wrapper();

        ~Wrapper();

        bool init();

        bool listen(uint16_t port);

        bool connect(const char *host, uint16_t port);

        void disconnect();

        void service(uint32_t timeout, EventCallback callback);

        void send(const uint8_t *data, size_t size); // Send to connected peer/broadcast
        void send(ENetPeer *peer, const uint8_t *data, size_t size); // Send to specific peer

    private:
        struct Impl;
        Impl *pImpl;
    };
}
