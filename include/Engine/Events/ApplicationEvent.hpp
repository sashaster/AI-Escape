#pragma once
#include <Engine/Events/Event.hpp>

namespace Engine {

    class WindowCloseEvent final: public Event
    {
    public:
        EVENT_TYPE(WindowClose)

    };

    class WindowResizeEvent final: public Event {
    public:
        WindowResizeEvent(const int width, const int height): m_Width(width), m_Height(height) {}
        EVENT_TYPE(WindowResize)

        [[nodiscard]] int GetWidth() const {
            return m_Width;
        }
        [[nodiscard]] int GetHeight() const {
            return m_Height;
        }

    private:
        int m_Width, m_Height;
    };

    class FramebufferResizeEvent: public Event {
    public:
        FramebufferResizeEvent(const int width, const int height): m_Width(width), m_Height(height){}
        EVENT_TYPE(FramebufferResize)

        [[nodiscard]] int GetWidth() const {
            return m_Width;
        }
        [[nodiscard]] int GetHeight() const {
            return m_Height;
        }

    private:
        int m_Width, m_Height;
    };
}