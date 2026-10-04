#pragma once
#include <netinet/in.h>
#include <string_view>
#include <cstdint>
#include <string>
#include <tuple>


namespace Engine {

    class Net {
    public:
        static void Shutdown();
        static void Init();
        static bool Send(std::string_view message);
        static std::tuple<bool, std::string, sockaddr_in> Receive();

    private:
        static bool IsReady();
        inline static int s_Socket = -1;
        inline static sockaddr_in s_Hints;


    };
}