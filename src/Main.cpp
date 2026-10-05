#include <array>


#include <Engine/Core/Entry_Point.hpp>
#include <Engine/Core/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Scene/Maze.hpp>
#include <Scene/Agent.hpp>


static std::array<Engine::Vertex, 8> s_Lines;
static glm::vec4 s_LineColor = {1.0f, 1.0f, 1.0f, 1.0f};

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

        const auto [up, right, down, left] = Engine::Physics::RayCast(m_Agent.GetBox(), m_Maze.GetData().walls);
        TRACE("UP: {} RIGHT: {} DOWN: {} LEFT: {}", up.distance, right.distance, down.distance, left.distance);
        m_AgentSprite.transform = m_Agent.GetTransform();
        Engine::Renderer::Clear();
        Engine::Renderer::Draw(m_MazeSprite, m_CameraController.GetCamera());
        Engine::Renderer::Draw(m_AgentSprite, m_CameraController.GetCamera());

        for (auto i = 0; const auto& ray: {up, right, down, left}) {
            s_Lines[i++] = {.position = {ray.start.x, ray.start.y, 0.0f}, .color = s_LineColor};
            s_Lines[i++] = {.position = {ray.end.x, ray.end.y, 0.0f}, .color = s_LineColor};
        }
        m_LinesSprite.VA->GetBuffers()
                     .front()->SetData(s_Lines);
        Engine::Renderer::Draw(m_LinesSprite, m_CameraController.GetCamera(), true);
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

private:
    Engine::Sprite m_MazeSprite;
    Engine::Sprite m_AgentSprite;
    Engine::Sprite m_LinesSprite;
    Scene::Maze m_Maze;
    Scene::Agent m_Agent;
    Engine::OrthographicCameraController m_CameraController;

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
            {.position = { -0.5f,  -0.5f, 0.0f}, .color = {1.0f, 1.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, 0.0f}, .color = {1.0f, 1.0f, 0.0f, 1.0f}},
            {.position = {0.5, 0.5f, 0.0f}, .color = {1.0f, 1.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 0.0f, 1.0f}}
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