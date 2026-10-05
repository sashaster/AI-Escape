#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

#include <Engine/Core/Net.hpp>
#include <Engine/Core/Log.hpp>
#include <Engine/Core/Core.hpp>


namespace Engine {

    void Net::Init() {
        INFO("Starting server...");
        s_Hints.sin_family = AF_INET;
        s_Hints.sin_port = htons(ServerPort);
        s_Hints.sin_addr.s_addr = htonl(INADDR_ANY);

        s_Socket = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT(s_Socket >= 0, "Failed to create a socket: {}", std::strerror(errno))

        ASSERT(bind(s_Socket, reinterpret_cast<sockaddr*>(&s_Hints), sizeof(s_Hints)) >= 0, "Failed to assign address to a socket: {}", std::strerror(errno))
        INFO("Server started");
    }

    bool Net::Send(const std::string_view message) {

        return true;
    }

    bool Net::IsReady() {
        pollfd p{};
        p.fd = s_Socket;
        p.events = POLL_IN;
        const auto ready = poll(&p, 1, 0);
        if (ready < 0) {
            ERROR(std::strerror(errno));
            return false;
        }
        return (ready > 0) && (p.events & POLL_IN);
    }

    std::tuple<bool, std::string, sockaddr_in> Net::Receive() {
        std::string data(1024, ' ');
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        if (IsReady()) {
            const auto bytes = recvfrom(s_Socket, data.data(), data.size(), 0, reinterpret_cast<sockaddr*>(&client), &len);
            if (bytes < 0) {
                ERROR(std::strerror(errno));
                return {false, data, client};
            }
            data.resize(bytes);
            return {true, data, client};
        }

        return {false, data, client};
    }

    void Net::Shutdown() {
        if (s_Socket >= 0) {
            close(s_Socket);
        }
        INFO("Server shut down");
    }
}