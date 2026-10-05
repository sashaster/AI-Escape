#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>

#include <Engine/Core/Physics.hpp>


namespace Scene {
    class Agent {
    public:
        explicit Agent(const glm::vec2 position = glm::vec2(0.0f), const float size = 1.0f)
        : m_Position(position), m_Size(size){}

        [[nodiscard]] glm::mat4 GetTransform() const{
            return glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(m_Size, m_Size, 1.0f));
        }

        [[nodiscard]] glm::vec2 GetPosition() const {
            return m_Position;
        }

        [[nodiscard]] Engine::Physics::AABB GetBox() const {
            return {.min = {m_Position - m_Size * 0.5f}, .max = {m_Position + m_Size * 0.5f}};
        }

        void OnUpdate(float dt, std::span<const Engine::Physics::AABB> walls);
        void SetPosition(const glm::vec2 position) {
            m_Position = position;
        }
        static glm::vec2 ReadInput(float dt);

    private:
        glm::vec2 m_Position;
        float m_Size;
    };
}