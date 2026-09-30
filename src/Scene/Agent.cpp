#include <Scene/Agent.hpp>
#include <Engine/Input.hpp>


namespace Scene {
    namespace Input = Engine::Input;
    namespace Phys = Engine::Physics;

    void Agent::OnUpdate(const float dt, const std::vector<Phys::AABB>& walls, const Phys::AABB& exit) {
        const auto velocity = 20.0f * dt;
        glm::vec2 delta = {0.0f, 0.0f};

        if (Input::IsKeyPressed(GLFW_KEY_W)) {
            delta.y += velocity;
        }
        if (Input::IsKeyPressed(GLFW_KEY_S)) {
            delta.y -= velocity;
        }
        if (Input::IsKeyPressed(GLFW_KEY_D)) {
           delta.x += velocity;
        }
        if (Input::IsKeyPressed(GLFW_KEY_A)) {
            delta.x -= velocity;
        }

        m_Position.x += delta.x;
        if (Phys::Overlaps(GetBox(), exit)) {
            m_Escape = true;
            m_Position.x -= delta.x;
            return;
        }
        if (Phys::Overlaps(GetBox(), walls)) {
            m_Position.x -= delta.x;
        }

        m_Position.y += delta.y;

        if (Phys::Overlaps(GetBox(), exit)) {
            m_Escape = true;
            m_Position.y -= delta.y;
            return;
        }

        if (Phys::Overlaps(GetBox(), walls)) {
            m_Position.y -= delta.y;
        }
    }

}