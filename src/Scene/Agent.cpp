#include <Scene/Agent.hpp>


namespace Scene {
    using namespace Engine::Physics;
    static constexpr float kAgentSpeed = 100.0f;

    //TODO: check collisions only for walls near the agent
    bool Agent::OnUpdate(const float dt, const std::span<const AABB> walls, glm::vec2 dir) {
        if (glm::dot(dir, dir) == 0) {
            return false;
        }

        dir = glm::normalize(dir);
        const auto moveDelta = dir * kAgentSpeed * dt;
        const auto hitX = Move(0, moveDelta.x, walls);
        const auto hitY = Move(1, moveDelta.y, walls);

        return hitX || hitY;
    }

    bool Agent::Move(const int axis, const float moveDelta, const std::span<const AABB> walls) {
        if (moveDelta == 0.0f) {
            return false;
        }
        m_Position[axis] += moveDelta;
        const auto half = m_Size * 0.5f;
        auto hit = false;

        for (const auto& wall: walls) {
            if (Overlaps(Box(), wall)) {

                if (moveDelta > 0.0f) {
                    m_Position[axis] = wall.min[axis] - half;
                }
                else {
                    m_Position[axis] = wall.max[axis] + half;
                }
                hit = true;
            }
        }
        return hit;
    }
}
