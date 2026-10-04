#pragma once
#include <spdlog/spdlog.h>


namespace Engine {
    class Log {
    public:
        static void Init();
        static std::shared_ptr<spdlog::logger>& GetLogger() {
            return s_Logger;
        }

    private:
        inline static std::shared_ptr<spdlog::logger> s_Logger;
    };
}


#define TRACE(...)  Engine::Log::GetLogger()->trace(__VA_ARGS__)
#define DEBUG(...)  Engine::Log::GetLogger()->debug(__VA_ARGS__)
#define INFO(...)   Engine::Log::GetLogger()->info(__VA_ARGS__)
#define WARN(...)   Engine::Log::GetLogger()->warn(__VA_ARGS__)
#define ERROR(...)  Engine::Log::GetLogger()->error(__VA_ARGS__)
#define FATAL(...)  Engine::Log::GetLogger()->critical(__VA_ARGS__)
