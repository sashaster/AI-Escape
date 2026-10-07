#pragma once
#include <Engine/Events/Event.hpp>


namespace Engine {

    class Layer {
    public:
        virtual ~Layer() = default;
        virtual void OnEvent(Event& event){};
        virtual void OnUpdate(float dt) {}
        virtual void OnImGuiRender(){};
    };
}