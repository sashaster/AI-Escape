#include <Scene/Agent.hpp>
#include <Engine/Core/Input.hpp>


namespace Scene {

    static constexpr float kAgentSpeed = 20.0f;

    glm::vec2 Agent::ReadInput(const float dt) {
        glm::vec2 dir{0.0f, 0.0f};

        if (Engine::Input::IsKeyPressed(GLFW_KEY_W)) {
            dir.y += 1.0f;
        }
        if (Engine::Input::IsKeyPressed(GLFW_KEY_S)) {
            dir.y -= 1.0f;
        }
        if (Engine::Input::IsKeyPressed(GLFW_KEY_D)) {
            dir.x += 1.0f;
        }
        if (Engine::Input::IsKeyPressed(GLFW_KEY_A)) {
            dir.x -= 1.0f;
        }

        if (glm::dot(dir, dir) > 0) {
            dir = glm::normalize(dir);
        }

        return dir * kAgentSpeed * dt;
    }

    void Agent::OnUpdate(const float dt, const std::vector<Engine::Physics::AABB>& walls) {
        const auto moveDelta = ReadInput(dt);
        const auto half = m_Size * 0.5f;
        m_Position.x += moveDelta.x;
        for (const auto& wall: walls) {
            if (Engine::Physics::Overlaps(GetBox(), wall)) {
                if (moveDelta.x > 0.0f) {
                    m_Position.x = wall.min.x - half;
                }
                else if (moveDelta.x < 0.0f) {
                    m_Position.x = wall.max.x + half;
                }
            }
        }

        m_Position.y += moveDelta.y;
        for (const auto& wall: walls) {
            if (Engine::Physics::Overlaps(GetBox(), wall)) {
                if (moveDelta.y > 0.0f) {
                    m_Position.y = wall.min.y - half;
                }
                else if (moveDelta.y < 0.0f) {
                    m_Position.y = wall.max.y + half;
                }
            }
        }
    }
}