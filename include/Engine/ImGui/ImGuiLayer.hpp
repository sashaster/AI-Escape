#pragma once
#include <Engine/Core/Layer.hpp>
#include <Engine/Events/MouseEvent.hpp>
#include <Engine/Events/ApplicationEvent.hpp>

namespace Engine {

    class ImGuiLayer final: public Layer {
    public:
        void OnEvent(Event& event) override;
        ImGuiLayer();
        ~ImGuiLayer() override;
        void Begin(float dt);
        void End();
        void OnImGuiRender() override;

    private:
        bool OnMouseMoved(MouseMovedEvent& event);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
        bool OnMouseScrolled(MouseScrolledEvent& event);
        void UpdateDisplaySize();
        void DisplayDebug();
    };


}