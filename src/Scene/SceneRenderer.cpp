#include <Scene/SceneRenderer.hpp>
#include <Engine/Core/Physics.hpp>

#include <Scene/Agent.hpp>


namespace Scene {

    static struct MazeData {
        std::vector<Engine::Vertex> vertices;
        std::vector<std::uint32_t> indices;
    } s_MazeData;

    constexpr glm::vec4 kAgentColor = {1.0f, 1.0f, 0.0f, 1.0f};
    static std::array<Engine::Vertex, 16> s_SensorVertices;
    constexpr glm::vec4 kSensorColor = {1.0f, 1.0f, 1.0f, 1.0f};

    void SceneRenderer::BuildMaze(const Maze& maze) {
        constexpr auto wallColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        constexpr auto floorColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        constexpr auto exitColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        const auto cs = maze.GetCellSize();
        const auto wt = maze.GetWallThickness();
        const auto& grid = maze.GetGrid();
        const auto exit = maze.GetExitCell();
        const auto origin = maze.Origin();
        AddWall(origin.x + maze.GetWidth() * 0.5f, origin.y + maze.GetHeight() * 0.5f, maze.GetWidth() * 0.5f, maze.GetHeight() * 0.5f, floorColor);
        for (auto y = 0; y < grid.GetHeight(); ++y) {
            for (auto x = 0; x < grid.GetWidth(); ++x) {
                const auto &cell = grid[x, y];
                const auto cellPos = maze.GridToWorld(x, y);
                if (cell.wallN) {
                    AddWall(cellPos.x + cs * 0.5f,  cellPos.y + cs,  cs * 0.5f, wt * 0.5f, wallColor);
                }

                if (exit.x == x && exit.y == y) {
                    const float topY = cellPos.y + cs;
                    const float ecy = topY + cs * 0.5f;
                    AddWall(cellPos.x + cs * 0.5f, ecy, cs * 0.5f, cs * 0.5f, exitColor);
                }
                if (cell.wallE) {
                    AddWall(cellPos.x + cs, cellPos.y + cs * 0.5f, wt * 0.5f, cs* 0.5f,
                            wallColor);
                }
                if (y == 0 && cell.wallS) {
                    AddWall(cellPos.x + cs * 0.5f, cellPos.y, cs * 0.5f, wt * 0.5f, wallColor);
                }
                if (x == 0 && cell.wallW) {
                    AddWall(cellPos.x, cellPos.y + cs * 0.5f, wt * 0.5f, cs * 0.5f, wallColor);
                }
            }
        }
    }

    void SceneRenderer::AddWall(const float cx, const float cy, const float halfW, const float halfH, const glm::vec4 color) {
        const std::uint32_t base = s_MazeData.vertices.size();
        s_MazeData.vertices.emplace_back(glm::vec3(cx - halfW, cy - halfH, 0.0f), color);
        s_MazeData.vertices.emplace_back(glm::vec3(cx - halfW, cy + halfH, 0.0f), color);
        s_MazeData.vertices.emplace_back(glm::vec3(cx + halfW, cy + halfH, 0.0f), color);
        s_MazeData.vertices.emplace_back(glm::vec3(cx + halfW, cy - halfH, 0.0f), color);
        s_MazeData.indices.insert(s_MazeData.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
    }

    SceneRenderer::SceneRenderer(const Maze& maze): m_MazeSeed(maze.GetSeed()) {
        BuildMaze(maze);
        Init();
    }

    void SceneRenderer::Init() {
        const auto mazeVB = Engine::VertexBuffer::Create(s_MazeData.vertices.size());
        const auto mazeIB = Engine::IndexBuffer::Create(s_MazeData.indices);
        mazeVB->SetData(s_MazeData.vertices);
        const auto mazeVA = Engine::VertexArray::Create();
        mazeVA->AddVertexBuffer(mazeVB);
        mazeVA->SetIndexBuffer(mazeIB);
        m_Maze.VA = mazeVA;
        const auto shader = Engine::Shader::Create("Assets/Shaders/Color.glsl");
        m_Maze.shader = shader;

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
        m_Agent.VA = quadVA;
        m_Agent.shader = shader;

        const auto sensorVB = Engine::VertexBuffer::Create(s_SensorVertices.size());
        const auto sensorVA = Engine::VertexArray::Create();
        sensorVA->AddVertexBuffer(sensorVB);
        m_Sensor.VA = sensorVA;
        m_Sensor.shader = shader;
    }

    void SceneRenderer::Draw() {
        Engine::Renderer::Clear();
        Engine::Renderer::Draw(m_Maze, m_ViewProjection);
        Engine::Renderer::Draw(m_Agent, m_ViewProjection);
        if (m_Sensors) {
            Engine::Renderer::Draw(m_Sensor, m_ViewProjection, true);
        }
    }

    void SceneRenderer::Submit(const Agent& agent, const Maze& maze, const Engine::OrthographicCamera& camera, const std::span<const Engine::Physics::Ray> rays, const bool sensors) {
        if (m_MazeSeed != maze.GetSeed()) {
            s_MazeData.vertices.clear();
            s_MazeData.indices.clear();
            BuildMaze(maze);
            m_Maze.VA->GetBuffers().front()->SetData(s_MazeData.vertices);
        }
        m_Agent.transform = agent.Transform();
        m_ViewProjection = camera.GetViewProjection();
        m_Sensors = sensors;
        if (m_Sensors) {
            for (auto i = 0; const auto& ray: rays) {
                s_SensorVertices[i++] = {.position = {ray.start.x, ray.start.y, 0.0f}, .color = kSensorColor};
                s_SensorVertices[i++] = {.position = {ray.end.x, ray.end.y, 0.0f}, .color = kSensorColor};
            }
            m_Sensor.VA->GetBuffers()
                         .front()->SetData(s_SensorVertices);
        }
    }
}
