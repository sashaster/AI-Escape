#include <Scene/Agent.hpp>
#include <Engine/Core/Input.hpp>


namespace Scene {

    static constexpr float kAgentSpeed = 20.0f;
    void Agent::OnUpdate(const float dt, const std::vector<Engine::Physics::AABB>& walls) {

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

        const glm::vec2 delta = dir * kAgentSpeed * dt;

        m_Position.x += delta.x;
        if (Engine::Physics::Overlaps(GetBox(), walls)) {
            m_Position.x -= delta.x;
        }

        m_Position.y += delta.y;

        if (Engine::Physics::Overlaps(GetBox(), walls)) {
            m_Position.y -= delta.y;
        }
    }
}