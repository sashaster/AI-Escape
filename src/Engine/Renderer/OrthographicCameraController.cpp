#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Engine/Log.hpp>


namespace Engine {

    OrthographicCameraController::OrthographicCameraController(const float aspect, const float speed): m_Camera(-aspect, aspect, -1.0f, 1.0f), m_Speed(speed), m_Aspect(aspect) {
        m_Position = m_Camera.GetPosition();
    }

    void OrthographicCameraController::OnUpdate(const float dt) {

    }

    bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& event) {
        m_Aspect = static_cast<float>(event.GetWidth()) / static_cast<float>(event.GetHeight());
        m_Camera.SetProjection(-m_Aspect, m_Aspect, -1.0f, 1.0f);
        return false;
    }

    void OrthographicCameraController::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
            return OnWindowResize(e);
        });
    }
}
