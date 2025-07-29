#pragma once
#include <cstdint>
#include <functional>

namespace Net {
    enum class EventType { Connect, Receive, Disconnect };

    struct Event {
        EventType type;
        const uint8_t *data;
        size_t dataLength;
    };

    class Wrapper {
    public:
        Wrapper();

        ~Wrapper();

        bool init();

        bool connect(const char *host, uint16_t port);

        void service(unsigned int timeoutMs, std::function<void(const Event &)> callback);

        void disconnect();

        void send(const uint8_t *data, size_t len);

        bool listen(uint16_t port);

    private:
        struct Impl;
        Impl *pImpl;
    };
}
