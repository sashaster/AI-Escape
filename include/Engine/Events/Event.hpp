#pragma once
#include <concepts>
#include <functional>

namespace Engine {

    enum class EventType{
        None,
        WindowClose,
        WindowResize,
        KeyPressed
    };

#define EVENT_TYPE(type) \
    [[nodiscard]] static EventType GetEventType() { return EventType::type;} \
    [[nodiscard]] EventType GetType() const override { return GetEventType(); }

    class Event {
        friend class EventDispatcher;
    public:
        virtual ~Event() = default;
        [[nodiscard]] virtual EventType GetType() const = 0;
        [[nodiscard]] bool IsHandled() const {
            return m_Handled;
        }

    protected:
        bool m_Handled = false;

    };

    class EventDispatcher {
        template<typename T>
        using EventFn = std::function<bool(T&)>;
    public:
        explicit EventDispatcher(Event& event): m_Event(event) {}

        template<typename T>
            requires std::derived_from<T, Event>
        bool Dispatch(const EventFn<T>& fn) {
            if (m_Event.GetType() == T::GetEventType()) {
                m_Event.m_Handled = fn(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };
}