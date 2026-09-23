#pragma once
#include <Engine/Renderer/OrthographicCamera.hpp>
#include <Engine/Events/ApplicationEvent.hpp>


namespace Engine {

    class OrthographicCameraController {
    public:
        explicit OrthographicCameraController(float aspect, float speed = 1.0);

        void OnUpdate(float dt);
        void OnEvent(Event& event);
        [[nodiscard]] const OrthographicCamera& GetCamera() const {
            return m_Camera;
        }
    private:
        bool OnWindowResize(WindowResizeEvent& event);
        OrthographicCamera m_Camera;
        float m_Speed;
        glm::vec3 m_Position;
        float m_Aspect;

    };

}