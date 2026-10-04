#pragma once
#include <csignal>
#include <glad/gl.h>
#include <cstdint>

#include <Engine/Core/Log.hpp>

namespace Engine {
    using RendererID = GLuint;
    inline constexpr int WindowWidth = 1280;
    inline constexpr int WindowHeight = 720;
    inline constexpr std::uint16_t ServerPort = 8080;
}

#ifndef NDEBUG
    #ifdef _MSC_VER
        #define BREAKPOINT() __debugbreak()
    #elif __clang__
        #define BREAKPOINT() __builtin_debugtrap()
    #elif __GNUC__
        #define BREAKPOINT() raise(SIGTRAP)
    #else
        #error "Unsupported compiler"
    #endif
#else
    #define BREAKPOINT()
#endif

#ifndef NDEBUG
    #define ASSERT(condition, ...) { if (!(condition)) { FATAL(__VA_ARGS__); BREAKPOINT(); } }
#else
    #define ASSERT(condition, ...)
#endif
