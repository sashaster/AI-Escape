#pragma once
#include <Engine/Events/Event.hpp>


namespace Engine {
    class KeyEvent : public Event {
    public:
        [[nodiscard]] int GetKeyCode() const {
            return m_KeyCode;
        }

    protected:
        explicit KeyEvent(const int keycode) : m_KeyCode(keycode) {
        }

        int m_KeyCode;
    };

    class KeyPressedEvent final : public KeyEvent {
    public:
        explicit KeyPressedEvent(const int keycode) : KeyEvent(keycode) {}

        EVENT_TYPE(KeyPressed)
    };

    class KeyReleasedEvent final : public KeyEvent {
    public:
        explicit KeyReleasedEvent(const int keycode) : KeyEvent(keycode) {}

        EVENT_TYPE(KeyReleased)
    };
}
