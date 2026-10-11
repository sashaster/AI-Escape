#include <array>
#include <imgui.h>
#include <unordered_map>

#include <Engine/Core/Entry_Point.hpp>
#include <Engine/Core/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Scene/Maze.hpp>
#include <Scene/Agent.hpp>
#include <Scene/SceneRenderer.hpp>


constexpr float kMaxDistance = 5.0f;
static std::unordered_map<std::string, Engine::Physics::Ray> s_Rays;

struct Sensor {
    std::string name;
    float angle;
};
constexpr std::array kAngles = {90.0f, 45.0f, 0.0f, 315.0f, 270.0f, 225.0f, 180.0f, 135.0f};
const std::array<Sensor, 8> directions = {
    {{.name = "up", .angle = 90.0f}, {.name = "upRight", .angle = 45.0f},
    {.name = "right", .angle = 0.0f}, {.name = "downRight", .angle = 315.0f},
    {.name = "down", .angle = 270.0f}, {.name = "downLeft", .angle = 225.0f},
    {.name = "left", .angle = 180.0f}, {.name = "upLeft", .angle = 135.0f}
    }
};

constexpr glm::ivec2 kGridDim{10, 10};
constexpr glm::ivec2 kExitCell{kGridDim.x - 1, kGridDim.y - 1};

void RayBar(const std::string_view name, const float distance, const float maxDist) {
    const auto d = distance / maxDist;
    const auto color = d <= 0.25f ? ImVec4(0.8f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
    const auto overlay = std::format("{:.2f}", distance);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(d, ImVec2(0.0f, 0.0f), overlay.data());
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextUnformatted(name.data());
}

class Simulation final: public Engine::Layer {
public:
    Simulation():m_Maze(kGridDim.x, kGridDim.y, 10.0f, 0.5f, kExitCell),
                m_Agent(m_Maze[0, 0], m_Maze.GetCellSize() * 0.5f),
                m_Scene(m_Maze),
                m_CameraController(static_cast<float>(Engine::kWindowWidth)/Engine::kWindowHeight, 100.0f)
    {
        m_Rays.reserve(kAngles.size());
    }

    void OnUpdate(const float dt) override {
        if (m_Reset) {
            m_Maze.Generate(std::random_device{}());
            m_Agent.SetPosition(m_Maze[0, 0]);
        }

        auto message = Engine::Net::ReceiveAction();
        m_Agent.OnUpdate(dt, m_Maze.GetWalls());

        const auto exitReached = Engine::Physics::Overlaps(m_Agent.Box(), m_Maze.GetExit());
        m_Reset = exitReached;

        m_Rays = Engine::Physics::RayCast(m_Agent.Box(), m_Maze.GetWalls(), kAngles, m_SensorRange);
        Engine::json obs;
        obs["done"] = exitReached;
        const auto exitPos = m_Maze.GetExit().Center();
        const auto toExit = exitPos - m_Agent.GetPosition();
        m_ExitDist = glm::length(toExit);
        if (m_ExitDist > 0) {
            m_ExitDir = glm::normalize(toExit);
        }else {
            m_ExitDir = glm::vec2{0.0f};
        }
        obs["exit_dist"] = m_ExitDist/glm::length(m_Maze.Size());
        obs["exit_dir"] = {
                {"x", m_ExitDir.x},
                {"y", m_ExitDir.y}
            };

        for (auto i = 0; const auto& ray: m_Rays) {
            obs[directions[i].name] = ray.distance/m_SensorRange;
            s_Rays[directions[i].name] = ray;
            i++;
        }
        if (message) {
            message->data = obs;
            Engine::Net::SendObservation(message.value());
        }
        m_Scene.Submit(m_Agent, m_Maze, m_CameraController.GetCamera(), m_Rays, m_ShowSensors);
        m_Scene.Draw();
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Debug");
        ImGui::SeparatorText("Agent");
        {
            RayBar("Up", s_Rays["up"].distance, m_SensorRange);
            RayBar("Down",  s_Rays["down"].distance, m_SensorRange);
            RayBar("Right",  s_Rays["right"].distance, m_SensorRange);
            RayBar("Left",  s_Rays["left"].distance, m_SensorRange);
            ImGui::SliderFloat("Sensor range", &m_SensorRange, 1.0f, kMaxDistance, "%.1f");
            ImGui::Checkbox("Show sensors", &m_ShowSensors);
        }
        ImGui::SeparatorText("Exit");
        {
            ImGui::Text("Distance: %.2f (%.2f)", m_ExitDist, m_ExitDist/glm::length(m_Maze.Size()));
            ImGui::Text("Direction: (%.2f, %.2f)", m_ExitDir.x, m_ExitDir.y);
        }
        ImGui::End();
    }

private:
    Scene::Maze m_Maze;
    Scene::Agent m_Agent;
    Scene::SceneRenderer m_Scene;
    Engine::OrthographicCameraController m_CameraController;
    bool m_ShowSensors = false;
    float m_SensorRange = kMaxDistance;
    float m_ExitDist = 0.0f;
    bool m_Reset = false;
    glm::vec2 m_ExitDir;
    std::vector<Engine::Physics::Ray> m_Rays;
};


class AI_Escape: public Engine::Application {
public:
    AI_Escape() {
        PushLayer(std::make_unique<Simulation>());
    }
};

std::unique_ptr<Engine::Application> Engine::CreateApplication() {
    return std::make_unique<AI_Escape>();
}