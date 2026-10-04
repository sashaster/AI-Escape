#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Engine/Core/Log.hpp>


namespace Engine {

    OrthographicCameraController::OrthographicCameraController(const float aspect, const float zoomLevel ,const float speed): m_Camera(-aspect * zoomLevel, aspect * zoomLevel, -zoomLevel, zoomLevel), m_Speed(speed), m_Aspect(aspect), m_ZoomLevel(zoomLevel) {
        m_Position = m_Camera.GetPosition();
    }

    bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& event) {
        if (event.GetWidth() == 0 || event.GetHeight() == 0) {
            return false;
        }
        m_Aspect = static_cast<float>(event.GetWidth()) / static_cast<float>(event.GetHeight());
        m_Camera.SetProjection(-m_Aspect * m_ZoomLevel, m_Aspect * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    void OrthographicCameraController::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
            return OnWindowResize(e);
        });
    }
}
