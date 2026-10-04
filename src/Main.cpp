#include <array>


#include <Engine/Core/Entry_Point.hpp>
#include <Engine/Core/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>
#include <Scene/Maze.hpp>
#include <Scene/Agent.hpp>


class Simulation final: public Engine::Layer {
public:
    Simulation(): m_Maze(Scene::MazeGrid(10, 10), 10.0f, 0.5f),
                m_CameraController(static_cast<float>(Engine::WindowWidth)/Engine::WindowHeight, 100.0f)
    {
        UploadMaze();
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

        m_Agent = std::make_unique<Scene::Agent>(glm::vec3(m_Maze[0, 0], 0.0f), m_Maze.GetData().cellSize * 0.5f);
        m_AgentSprite.transform = m_Agent->GetTransform();
    }

    void OnUpdate(const float dt) override {
        m_Agent->OnUpdate(dt, m_Maze.GetData().walls);

        if (Engine::Physics::Overlaps(m_Agent->GetBox(), m_Maze.GetData().exit)) {
            RegenerateMaze();
            m_Agent->SetPosition(m_Maze[0, 0]);
        }
        m_AgentSprite.transform = m_Agent->GetTransform();
        Engine::Renderer::Clear();
        Engine::Renderer::Draw(m_MazeSprite, m_CameraController.GetCamera());
        Engine::Renderer::Draw(m_AgentSprite, m_CameraController.GetCamera());
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

private:
    Engine::Sprite m_MazeSprite;
    Engine::Sprite m_AgentSprite;
    Scene::Maze m_Maze;
    std::unique_ptr<Scene::Agent> m_Agent;
    Engine::OrthographicCameraController m_CameraController;

    void RegenerateMaze() {
        m_Maze.Regenerate(std::random_device{}());
        UploadMaze();
    }

    void UploadMaze() {
        const auto mazeVB = Engine::VertexBuffer::Create(m_Maze.GetData().vertices);
        const auto mazeIB = Engine::IndexBuffer::Create(m_Maze.GetData().indices);
        const auto mazeVA = Engine::VertexArray::Create();
        mazeVA->AddVertexBuffer(mazeVB);
        mazeVA->SetIndexBuffer(mazeIB);
        m_MazeSprite.VA = mazeVA;
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
