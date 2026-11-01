#pragma once
#include <Engine/Renderer/OrthographicCamera.hpp>
#include <Engine/Events/ApplicationEvent.hpp>


namespace Engine {
    class OrthographicCameraController {
    public:
        explicit OrthographicCameraController(float aspect, float zoomLevel = 1.0f, float speed = 1.0);

        void OnEvent(Event& event);
        [[nodiscard]] const OrthographicCamera& GetCamera() const {
            return m_Camera;
        }
    private:
        bool OnWindowResize(WindowResizeEvent& event);
        OrthographicCamera m_Camera;
        float m_Speed;
        glm::vec2 m_Position;
        float m_Aspect;
        float m_ZoomLevel;

    };

}