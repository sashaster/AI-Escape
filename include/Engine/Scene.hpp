#pragma once
#include <Engine/Events/Event.hpp>

namespace Engine {


    class Scene {
    public:
        virtual ~Scene() = default;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnEvent(Event& event) = 0;
    };

}