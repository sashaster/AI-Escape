#pragma once
#include <Engine/Events/Event.hpp>


namespace Engine {
    class MouseMovedEvent final : public Event {
    public:
        MouseMovedEvent(const float x, const float y) : m_X(x), m_Y(y) {
        }

        EVENT_TYPE(MouseMoved)

        [[nodiscard]] float GetX() const {
            return m_X;
        }

        [[nodiscard]] float GetY() const {
            return m_Y;
        }

    private:
        float m_X, m_Y;
    };

    class MouseScrolledEvent final : public Event {
    public:
        MouseScrolledEvent(const float xOffset, const float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {
        }

        EVENT_TYPE(MouseScrolled)

        [[nodiscard]] float GetXOffset() const {
            return m_XOffset;
        }

        [[nodiscard]] float GetYOffset() const {
            return m_YOffset;
        }

    private:
        float m_XOffset, m_YOffset;
    };

    class MouseButtonEvent : public Event {
    public:
        [[nodiscard]] int GetMouseButton() const {
            return m_Button;
        }

    protected:
        explicit MouseButtonEvent(const int button) : m_Button(button) {
        }

        int m_Button;
    };

    class MouseButtonPressedEvent final : public MouseButtonEvent {
    public:
        explicit MouseButtonPressedEvent(const int button) : MouseButtonEvent(button) {}

        EVENT_TYPE (MouseButtonPressed)
    };

    class MouseButtonReleasedEvent final : public MouseButtonEvent {
    public:
        explicit MouseButtonReleasedEvent(const int button) : MouseButtonEvent(button) {}

        EVENT_TYPE (MouseButtonReleased)
    };
}
