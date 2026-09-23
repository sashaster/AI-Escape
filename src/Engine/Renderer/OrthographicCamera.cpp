#include <Engine/Renderer/OrthographicCamera.hpp>


namespace Engine {

    OrthographicCamera::OrthographicCamera(const float left, const float right, const float bottom, const float top): m_Projection(glm::ortho(left, right, bottom, top)), m_View(1.0f), m_Position(0.0f) {
        CalculateViewProjection();
    }

    void OrthographicCamera::CalculateViewProjection() {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position);
        m_View = glm::inverse(transform);
        m_ViewProjection = m_Projection * m_View;
    }

    void OrthographicCamera::SetPosition(const glm::vec3& position) {
        m_Position = position;
        CalculateViewProjection();
    }

    void OrthographicCamera::SetProjection(const float left, const float right, const float bottom, const float top) {
        m_Projection = glm::ortho(left, right, bottom, top);
        CalculateViewProjection();
    }
}
