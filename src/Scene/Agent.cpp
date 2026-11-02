#include <Scene/Agent.hpp>


namespace Scene {
    using namespace Engine::Physics;
    static constexpr float kAgentSpeed = 20.0f;

    //TODO: check collisions only for walls near the agent
    void Agent::OnUpdate(const float dt, const std::span<const AABB> walls, glm::vec2 dir) {
        if (glm::dot(dir, dir) > 0) {
            dir = glm::normalize(dir);
        }

        const auto moveDelta = dir * kAgentSpeed * dt;
        const auto half = m_Size * 0.5f;
        m_Position.x += moveDelta.x;
        for (const auto& wall: walls) {
            if (Overlaps(Box(), wall)) {
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
            if (Overlaps(Box(), wall)) {
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
