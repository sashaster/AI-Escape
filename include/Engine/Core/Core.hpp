#pragma once
#include <csignal>
#include <glad/gl.h>
#include <cstdint>

#include <Engine/Core/Log.hpp>

namespace Engine {

    class Stats {
    public:
        void Update(const float dt) {
            m_Time += dt;
            ++m_Frames;

            if (m_Time >= 1.0f) {
                m_FPS = static_cast<float>(m_Frames) / m_Time;
                m_FrameTime = m_Time / static_cast<float>(m_Frames);
                m_Time = 0.0f;
                m_Frames = 0;
            }
        }

        [[nodiscard]] float GetFPS() const {
            return m_FPS;
        }

        [[nodiscard]] float GetFrameTime() const {
            return m_FrameTime * 1000.0f;
        }

    private:
        float m_Time = 0.0f;
        int m_Frames = 0;
        float m_FPS = 0.0f;
        float m_FrameTime = 0.0f;

    };

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
