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
#include <Engine/Core/Random.hpp>


constexpr float kMaxDistance = 5.0f;
constexpr float kAgentDt = 1.0f/60.0f;
constexpr std::array kDirections = {
    "upLeft", "up", "upRight",
    "left", "right",
    "downLeft", "down", "downRight"
};



constexpr glm::ivec2 kGridDim{5, 5};
constexpr float kCellSize = 10.0f;
constexpr float kWallThickness = 0.5f;

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
    Simulation():m_Maze(kGridDim.x, kGridDim.y, kCellSize, kWallThickness, 0),
                m_Agent(m_Maze.CellCenter({0, 0}), m_Maze.GetCellSize() * 0.5f),
                m_Scene(m_Maze),
                m_CameraController(static_cast<float>(Engine::kWindowWidth)/Engine::kWindowHeight, 100.0f),
                m_Visited(m_Maze.GetGrid().GetWidth() * m_Maze.GetGrid().GetHeight(), 0)
    {
        
    }

    void OnUpdate(const float dt) override {
        Step();
        m_Scene.Submit(m_Agent, m_Maze, m_CameraController.GetCamera(), m_Rays, m_ShowSensors);
        m_Scene.Draw();
        m_Obs.clear();
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Debug");
        ImGui::SeparatorText("Agent");
        {
            RayBar("Up", m_Sensors["up"].distance, m_SensorRange);
            RayBar("Down",  m_Sensors["down"].distance, m_SensorRange);
            RayBar("Right",  m_Sensors["right"].distance, m_SensorRange);
            RayBar("Left",  m_Sensors["left"].distance, m_SensorRange);
            ImGui::SliderFloat("Sensor range", &m_SensorRange, 1.0f, kMaxDistance, "%.1f");
            ImGui::Checkbox("Show sensors", &m_ShowSensors);
            ImGui::Text("Cell: (%d, %d)", m_CurrentCell.x, m_CurrentCell.y);
        }
        ImGui::SeparatorText("Neighbors");
        {
            ImGui::Text("N: %s", m_Neighbors[0] == 1 ? "visited" : "not visited");
            ImGui::Text("E: %s", m_Neighbors[1] == 1 ? "visited" : "not visited");
            ImGui::Text("S: %s", m_Neighbors[2] == 1 ? "visited" : "not visited");
            ImGui::Text("W: %s", m_Neighbors[3] == 1 ? "visited" : "not visited");
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
    std::array<std::uint32_t, 4> m_Neighbors{};
    Engine::json m_Obs;
    std::array<Engine::Physics::Ray, 8> m_Rays{};
    glm::ivec2 m_CurrentCell{0};
    std::unordered_map<std::string, Engine::Physics::Ray> m_Sensors;
    std::vector<std::uint8_t> m_Visited;

    void Step() {
        if (auto message = Engine::Net::ReceiveAction(); message) {
            const auto& action = message->data["action"];
            const auto& reset = message->data["reset"];
            if(action.is_array() && action.size() == 2 && reset.is_boolean()){
                if(reset){
                    m_Maze.Generate(0);
                    m_Agent.SetPosition(m_Maze.CellCenter({0, 0}));
                    m_Visited.clear();
                    m_Visited.resize(m_Maze.GetGrid().GetWidth() * m_Maze.GetGrid().GetHeight());
                    std::fill(m_Visited.begin(), m_Visited.end(), 0);
                }
                m_Agent.OnUpdate(kAgentDt, m_Maze.GetWalls(), {action.at(0), action.at(1)});
                ComputeObservations();
                message->data = m_Obs;
                Engine::Net::SendObservation(message.value());
            }
            else {
                WARN("Invalid message");
            }
        }else {
            ComputeObservations();
        }
    }

    void ComputeObservations() {
        m_Rays = Engine::Physics::RayCast(m_Agent.Box(), m_Maze.GetWalls(), m_SensorRange);
        auto& state = m_Obs["state"];
        auto& sensors = state["sensors"];
        for (auto i = 0; const auto& ray: m_Rays) {
            sensors.push_back(ray.distance/m_SensorRange);
            m_Sensors[kDirections[i]] = ray;
            i++;
        }
        m_CurrentCell = m_Maze.CellAt(m_Agent.GetPosition());
        m_Obs["cell"] = {m_CurrentCell.x, m_CurrentCell.y};
        auto& neighbors = state["neighbors"];
        auto i = 0;
        for (const auto visited: VisitedNeighbors(m_CurrentCell)) {
            neighbors.push_back(visited);
            m_Neighbors[i++] = visited;
        }

        const auto exitReached = Engine::Physics::Overlaps(m_Agent.Box(), m_Maze.GetExit());
        m_Obs["done"] = exitReached;
    }

    std::array<std::uint32_t, 4> VisitedNeighbors(const glm::uvec2 cell) {
        const auto& grid = m_Maze.GetGrid();
        const auto Index = [&grid](const std::uint32_t x, const std::uint32_t y){
            return grid.GetWidth() * y + x;
        };

        if (grid.InBounds(cell.x, cell.y)) {
            m_Visited.at(Index(cell.x, cell.y)) = 1;
        }
        std::array<std::uint32_t, 4> neighbors{1, 1, 1, 1};
        const auto x = cell.x, y = cell.y;
        if(grid.InBounds(x, y+1) && m_Visited.at(Index(x, y + 1)) == 0 && !grid[x, y].wallN){
            neighbors[0] = 0;
        }
        if(grid.InBounds(x+1, y) && m_Visited.at(Index(x+1, y)) == 0 && !grid[x, y].wallE){
            neighbors[1] = 0;
        }
        if(y > 0 && grid.InBounds(x, y-1) && m_Visited.at(Index(x, y-1)) == 0 && !grid[x, y].wallS){
            neighbors[2] = 0;
        }
        if(x > 0 && grid.InBounds(x-1, y) && m_Visited.at(Index(x-1, y)) == 0 && !grid[x, y].wallW){
            neighbors[3] = 0;
        }

        const auto exitCell = m_Maze.GetLastCell();
        if(exitCell == cell){
            neighbors[0] = 0;
        }
        
        return neighbors;
    }
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
