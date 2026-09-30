#pragma once
#include <Engine/Events/Event.hpp>


namespace Engine {


    class KeyPressedEvent final: public Event{
    public:
        explicit KeyPressedEvent(const int keyCode): m_KeyCode(keyCode){}
        [[nodiscard]] int GetKey() const noexcept {
            return m_KeyCode;
        }

        EVENT_TYPE(KeyPressed)

    private:
        int m_KeyCode;
    };

}