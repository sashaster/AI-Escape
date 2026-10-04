#include <spdlog/sinks/stdout_color_sinks.h>

#include <Engine/Core/Log.hpp>


namespace Engine {
    void Log::Init() {
        s_Logger = spdlog::stdout_color_mt("AI-Escape");
        s_Logger->set_level(spdlog::level::trace);
        s_Logger->set_pattern("%^%Y-%m-%d %T [%l] %n: %v%$");
        DEBUG("Logger initialized");
    }
}
