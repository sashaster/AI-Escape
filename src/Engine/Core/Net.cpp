#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

#include <Engine/Core/Net.hpp>
#include <Engine/Core/Log.hpp>
#include <Engine/Core/Core.hpp>


namespace Engine {
    constexpr std::uint16_t kServerPort = 8080;
    constexpr std::size_t kPacketSize = 65535;
    static std::string s_Buffer(kPacketSize, ' ');

    void Net::Init() {
        INFO("Starting server...");
        s_ServerAddr.sin_family = AF_INET;
        s_ServerAddr.sin_port = htons(kServerPort);
        s_ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        s_Socket = socket(AF_INET, SOCK_DGRAM, 0);
        VERIFY(s_Socket >= 0, "Failed to create a socket: {}", std::strerror(errno))
        VERIFY(bind(s_Socket, reinterpret_cast<sockaddr*>(&s_ServerAddr), sizeof(s_ServerAddr)) >= 0,
               "Failed to assign address to a socket: {}", std::strerror(errno))
        INFO("Server started");
    }

    bool Net::SendObservation(const Message &msg) {
        const auto j = msg.data.dump();
        auto addr = msg.from;
        if (sendto(s_Socket, j.data(), j.size(), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
            ERROR("sendto failed: {}", std::strerror(errno));
            return false;
        }
        return true;
    }

    bool Net::IsReady() {
        pollfd p{};
        p.fd = s_Socket;
        p.events = POLLIN;
        const auto ready = poll(&p, 1, 0);
        if (ready < 0) {
            ERROR(std::strerror(errno));
            return false;
        }
        return (ready > 0) && (p.events & POLLIN);
    }

    std::optional<Message> Net::ReceiveAction() {
        sockaddr_in from{};
        socklen_t len = sizeof(from);
        if (!IsReady()) {
            return std::nullopt;
        }

        const auto bytes = recvfrom(s_Socket, s_Buffer.data(), s_Buffer.size(), 0, reinterpret_cast<sockaddr *>(&from),
                                    &len);
        if (bytes < 0) {
            ERROR("recvfrom failed: {}", std::strerror(errno));
            return std::nullopt;
        }
        auto data = json::parse(s_Buffer.data(), s_Buffer.data() + bytes, nullptr, false);
        if (data.is_discarded()) {
            WARN("Received invalid JSON");
            return std::nullopt;
        }
        return Message{.data = std::move(data), .from = from};
    }

    void Net::Shutdown() {
        if (s_Socket >= 0) {
            if (close(s_Socket) < 0) {
                ERROR("close failed: {}", std::strerror(errno));
            }
            s_Socket = -1;
        }
        INFO("Server shut down");
    }
}
