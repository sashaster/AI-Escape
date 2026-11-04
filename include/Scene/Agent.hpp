#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Core/Physics.hpp>


namespace Scene {
    class Agent {
    public:
        Agent(const glm::vec2 position, const float size): m_Position(position), m_Size(size){}
        [[nodiscard]] glm::mat4 Transform() const{
            return glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(m_Size, m_Size, 1.0f));
        }

        [[nodiscard]] glm::vec2 GetPosition() const {
            return m_Position;
        }

        [[nodiscard]] float GetSize() const {
            return m_Size;
        }

        [[nodiscard]] Engine::Physics::AABB Box() const {
            return {.min = {m_Position - m_Size * 0.5f}, .max = {m_Position + m_Size * 0.5f}};
        }

        bool OnUpdate(float dt, std::span<const Engine::Physics::AABB> walls, glm::vec2 dir);
        void SetPosition(const glm::vec2 position) {
            m_Position = position;
        }

    private:
        bool Move(int axis, float moveDelta, std::span<const Engine::Physics::AABB> walls);
        glm::vec2 m_Position;
        float m_Size = 0.0f;
    };
}
