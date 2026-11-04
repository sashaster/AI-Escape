#include <array>
#include <deque>
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


constexpr float kMaxDistance = 10.0f;
constexpr float kAgentDt = 1.0f/60.0f;
constexpr std::array kDirections = {"up", "right", "down", "left"};


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
    Simulation():m_Maze(kGridDim.x, kGridDim.y, kCellSize, kWallThickness),
                m_Agent(m_Maze.CellCenter({0, 0}), m_Maze.GetCellSize() * 0.5f),
                m_Scene(m_Maze),
                m_CameraController(static_cast<float>(Engine::kWindowWidth)/Engine::kWindowHeight, 100.0f),
                m_Visited(m_Maze.GetGrid().GetWidth() * m_Maze.GetGrid().GetHeight(), 0)
    {

    }

    void OnUpdate(const float dt) override {
        Step();
        m_Scene.Submit(m_Agent, m_Maze, m_CameraController.GetCamera(), m_Sweeps, m_ShowSensors);
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
            for (const auto* name: kDirections) {
                RayBar(name, m_Sensors[name].distance, m_SensorRange);
            }
            ImGui::SliderFloat("Sensor range", &m_SensorRange, 1.0f, kMaxDistance, "%.1f");
            ImGui::Checkbox("Show sensors", &m_ShowSensors);
            ImGui::Text("Cell: (%d, %d)", m_CurrentCell.x, m_CurrentCell.y);
            ImGui::Text("Hit: %s", m_Hit ? "true" : "false");
            constexpr std::array kHintNames = {"N", "E", "S", "W"};
            auto hint = "none";
            for (std::size_t d = 0; d < m_Hint.size(); ++d) {
                if (m_Hint[d] == 1) hint = kHintNames[d];
            }
            ImGui::Text("Hint: %s", hint);
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
    std::array<std::uint32_t, 4> m_Hint{};
    Engine::json m_Obs;
    std::array<Engine::Physics::Sweep, 4> m_Sweeps{};
    glm::ivec2 m_CurrentCell{0};
    std::unordered_map<std::string, Engine::Physics::Sweep> m_Sensors;
    std::vector<std::uint8_t> m_Visited;
    bool m_Hit = false;
    bool m_Done = false;

    void Step() {
        if (auto message = Engine::Net::ReceiveAction(); message) {
            const auto& action = message->data["action"];
            const auto& reset = message->data["reset"];
            if(action.is_array() && action.size() == 2 && reset.is_boolean()){
                if(reset){
                    m_Maze.Generate(std::random_device{}());
                    m_Agent.SetPosition(m_Maze.CellCenter({0, 0}));
                    m_Visited.clear();
                    m_Visited.resize(m_Maze.GetGrid().GetWidth() * m_Maze.GetGrid().GetHeight());
                    std::fill(m_Visited.begin(), m_Visited.end(), 0);
                }
                m_Hit = m_Agent.OnUpdate(kAgentDt, m_Maze.GetWalls(), {action.at(0), action.at(1)});

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
        m_Sweeps = Engine::Physics::SweepCast(m_Agent.Box(), m_Maze.GetWalls(), m_SensorRange);
        auto& state = m_Obs["state"];
        auto& sensors = state["sensors"];
        for (auto i = 0; const auto& sweep: m_Sweeps) {
            sensors.push_back(sweep.distance/m_SensorRange);
            m_Sensors[kDirections[i]] = sweep;
            i++;
        }
        m_CurrentCell = m_Maze.CellAt(m_Agent.GetPosition());
        m_Obs["cell"] = {m_CurrentCell.x, m_CurrentCell.y};
        auto offset = glm::vec2(0.0f);
        if (const auto& grid = m_Maze.GetGrid(); grid.InBounds(m_CurrentCell.x, m_CurrentCell.y)) {
            m_Visited.at(grid.GetWidth() * m_CurrentCell.y + m_CurrentCell.x) = 1;
            offset = (m_Agent.GetPosition() - m_Maze.CellCenter(m_CurrentCell)) / m_Maze.GetCellSize();
        }
        state["offset"] = {offset.x, offset.y};
        m_Hint = NearestUnvisitedHint(m_CurrentCell);
        state["hint"] = m_Hint;

        const auto& exitBox = m_Maze.GetExit();
        const auto toExit = exitBox.Center() - m_Agent.GetPosition();
        state["exit"] = {toExit.x / m_Maze.GetWidth(), toExit.y / m_Maze.GetHeight()};

        m_Done = Engine::Physics::Overlaps(m_Agent.Box(), exitBox);
        m_Obs["done"] = m_Done;
        m_Obs["hit"] = m_Hit;
    }

    [[nodiscard]] std::array<std::uint32_t, 4> NearestUnvisitedHint(const glm::uvec2 start) const {
        std::array<std::uint32_t, 4> hint{0, 0, 0, 0};
        const auto& grid = m_Maze.GetGrid();
        if (!grid.InBounds(start.x, start.y)) {
            return hint;
        }
        const auto exitCell = m_Maze.GetLastCell();
        if (start == exitCell) {
            hint[0] = 1;
            return hint;
        }

        const auto width = grid.GetWidth();
        const auto Index = [width](const glm::uvec2 c) { return width * c.y + c.x; };
        constexpr std::array<glm::ivec2, 4> kSteps = {{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}};
        const auto IsOpen = [&grid](const glm::uvec2 c, const std::size_t dir) {
            const auto& cell = grid[c.x, c.y];
            const std::array open = {!cell.wallN, !cell.wallE, !cell.wallS, !cell.wallW};
            return open[dir];
        };

        std::vector firstDir(width * grid.GetHeight(), -1);
        firstDir[Index(start)] = 4;
        std::deque queue{start};

        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop_front();
            const auto currentIdx = Index(current);
            if (current != start && (m_Visited.at(currentIdx) == 0 || current == exitCell)) {
                hint[firstDir[currentIdx]] = 1;
                return hint;
            }
            for (std::size_t dir = 0; dir < kSteps.size(); ++dir) {
                if (!IsOpen(current, dir)) {
                    continue;
                }
                const auto next = glm::ivec2(current) + kSteps[dir];
                if (next.x < 0 || next.y < 0 || !grid.InBounds(next.x, next.y)) {
                    continue;
                }
                const auto nextIdx = Index(glm::uvec2(next));
                if (firstDir[nextIdx] != -1) {
                    continue;
                }
                firstDir[nextIdx] = current == start ? static_cast<int>(dir) : firstDir[currentIdx];
                queue.emplace_back(next);
            }
        }
        return hint;
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
