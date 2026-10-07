#include <array>
#include <imgui.h>

#include <Engine/Core/Entry_Point.hpp>
#include <Engine/Core/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Scene/Maze.hpp>
#include <Scene/Agent.hpp>


static std::array<Engine::Vertex, 16> s_Lines;
constexpr glm::vec4 kLineColor = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr glm::vec4 kAgentColor = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr float kMaxDistance = 5.0f;
static std::vector<Engine::Physics::Ray> s_Rays;

void RayBar(const std::string_view name, const float distance, const float maxDist = kMaxDistance) {
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
    Simulation(): m_Maze(Scene::MazeGrid(10, 10), 10.0f, 0.5f),
                m_Agent(m_Maze[0, 0], m_Maze.GetData().cellSize * 0.5f),
                m_CameraController(static_cast<float>(Engine::WindowWidth)/Engine::WindowHeight, 100.0f)
    {
        Setup();
    }

    void OnUpdate(const float dt) override {
        m_Agent.OnUpdate(dt, m_Maze.GetData().walls);
        if (Engine::Physics::Overlaps(m_Agent.GetBox(), m_Maze.GetData().exit)) {
            RegenerateMaze();
            m_Agent.SetPosition(m_Maze[0, 0]);
        }

        s_Rays = Engine::Physics::RayCast(m_Agent.GetBox(), m_Maze.GetData().walls, m_SensorRange);
        m_AgentSprite.transform = m_Agent.GetTransform();
        Engine::Renderer::Clear();
        Engine::Renderer::Draw(m_MazeSprite, m_CameraController.GetCamera());
        Engine::Renderer::Draw(m_AgentSprite, m_CameraController.GetCamera());
        if (m_ShowSensors) {
            for (auto i = 0; const auto& ray: s_Rays) {
                s_Lines[i++] = {.position = {ray.start.x, ray.start.y, 0.0f}, .color = kLineColor};
                s_Lines[i++] = {.position = {ray.end.x, ray.end.y, 0.0f}, .color = kLineColor};
            }
            m_LinesSprite.VA->GetBuffers()
                         .front()->SetData(s_Lines);
            Engine::Renderer::Draw(m_LinesSprite, m_CameraController.GetCamera(), true);
        }
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Debug");
        RayBar("Up", s_Rays[1].distance);
        RayBar("Down", s_Rays[6].distance);
        RayBar("Right", s_Rays[4].distance);
        RayBar("Left", s_Rays[3].distance);
        ImGui::SliderFloat("Sensor range", &m_SensorRange, 1.0f, kMaxDistance, "%.1f");
        ImGui::Checkbox("Show sensors", &m_ShowSensors);
        ImGui::End();
    }

private:
    Engine::Sprite m_MazeSprite;
    Engine::Sprite m_AgentSprite;
    Engine::Sprite m_LinesSprite;
    Scene::Maze m_Maze;
    Scene::Agent m_Agent;
    Engine::OrthographicCameraController m_CameraController;
    bool m_ShowSensors = false;
    float m_SensorRange = kMaxDistance;


    void RegenerateMaze() {
        m_Maze.Regenerate(std::random_device{}());
        m_MazeSprite.VA->GetBuffers()
                    .front()->SetData(m_Maze.GetData().vertices);
    }

    void Setup() {
        const auto mazeVB = Engine::VertexBuffer::Create(m_Maze.GetData().vertices.size());
        const auto mazeIB = Engine::IndexBuffer::Create(m_Maze.GetData().indices);
        mazeVB->SetData(m_Maze.GetData().vertices);
        const auto mazeVA = Engine::VertexArray::Create();
        mazeVA->AddVertexBuffer(mazeVB);
        mazeVA->SetIndexBuffer(mazeIB);
        m_MazeSprite.VA = mazeVA;
        const auto shader = Engine::Shader::Create("Assets/Shaders/Color.glsl");
        m_MazeSprite.shader = shader;

        std::array<Engine::Vertex, 4> quadVertices{{
            {.position = { -0.5f,  -0.5f, 0.0f},  .color = kAgentColor},
            {.position = {-0.5f, 0.5f, 0.0f},     .color = kAgentColor},
            {.position = {0.5, 0.5f, 0.0f},       .color = kAgentColor},
            {.position = {0.5f, -0.5f, 0.0f},     .color = kAgentColor}
        }};
        std::array<std::uint32_t, 6> quadIndices = {
            0, 1, 2, 2, 3, 0
        };

        const auto quadVB = Engine::VertexBuffer::Create(quadVertices);
        const auto quadIB = Engine::IndexBuffer::Create(quadIndices);
        const auto quadVA = Engine::VertexArray::Create();
        quadVA->AddVertexBuffer(quadVB);
        quadVA->SetIndexBuffer(quadIB);
        m_AgentSprite.VA = quadVA;
        m_AgentSprite.shader = shader;

        const auto linesVB = Engine::VertexBuffer::Create(s_Lines.size());
        const auto linesVA = Engine::VertexArray::Create();
        linesVA->AddVertexBuffer(linesVB);
        m_LinesSprite.VA = linesVA;
        m_LinesSprite.shader = shader;
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