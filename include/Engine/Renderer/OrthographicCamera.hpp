#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Engine {
    class OrthographicCamera {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);
        [[nodiscard]] const glm::mat4& GetViewProjection() const {
            return m_ViewProjection;
        }
        [[nodiscard]] const glm::vec2& GetPosition() const {
            return m_Position;
        }
        void SetPosition(glm::vec2 position);
        void SetProjection(float left, float right, float bottom, float top);

    private:
        void CalculateViewProjection();
        glm::mat4 m_Projection;
        glm::mat4 m_View;
        glm::mat4 m_ViewProjection;
        glm::vec2 m_Position;
    };

}