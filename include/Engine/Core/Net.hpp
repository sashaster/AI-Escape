#pragma once
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <optional>


namespace Engine {
    using json = nlohmann::json;


    struct Message {
        json data;
        sockaddr_in from;
    };

    class Net {
    public:
        static void Shutdown();
        static void Init();
        static bool SendObservation(const Message& msg);
        static std::optional<Message> ReceiveAction();

    private:
        static bool IsReady();
        inline static int s_Socket = -1;
        inline static sockaddr_in s_ServerAddr;


    };
}