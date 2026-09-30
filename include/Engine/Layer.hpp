#pragma once
#include <Engine/Events/Event.hpp>


namespace Engine {

    class Layer {
    public:
        virtual ~Layer() = default;
        virtual void OnEvent(Event& event) = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnAttach() = 0;

    };
}