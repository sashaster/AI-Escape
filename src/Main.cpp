#include <cstdint>
#include <stack>
#include <utility>
#include <random>


#include <Engine/Entry_Point.hpp>
#include <Engine/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>


struct MazeCell {
    bool visited = false;
    bool wallN = true, wallS = true, wallE = true, wallW = true;
};


class MazeGrid {
public:
    MazeGrid(const int width, const int height):  m_Width(width), m_Height(height), m_Cells(width*height) {
        ASSERT(m_Width > 0 && m_Height > 0, "Invalid grid dimensions")
    }
    const MazeCell& operator[](const int x, const int y) const {
        return m_Cells.at(y * m_Width + x);
    }

    MazeCell& operator[](const int x, const int y) {
        return m_Cells.at(y * m_Width + x);
    }

    [[nodiscard]] int GetWidth() const{
        return m_Width;
    }

    [[nodiscard]] int GetHeight() const {
        return m_Height;
    }

    void Generate() {
        auto device = std::random_device{};
        std::mt19937 generator(device());
        constexpr int startX = 0, startY = 0;
        std::stack<std::pair<int, int>> stack;
        (*this)[startX, startY].visited = true;
        stack.emplace(startX, startY);
        while (!stack.empty()) {
            auto [x, y] = stack.top();
            std::vector<std::pair<int, int>> neighbors;
            if (y < m_Height - 1 && !(*this)[x, y + 1].visited) { //N
                neighbors.emplace_back(x, y + 1);
            }
            if (y > 0 && !(*this)[x, y - 1].visited) { //S
                neighbors.emplace_back(x, y - 1);
            }
            if (x > 0 && !(*this)[x - 1, y].visited) { //W
                neighbors.emplace_back(x - 1, y);
            }
            if (x < m_Width - 1 && !(*this)[x + 1, y].visited) { //E
                neighbors.emplace_back(x + 1, y);
            }
            if (neighbors.empty()) {
                stack.pop();
                continue;
            }

            std::uniform_int_distribution<std::size_t> dist(0, neighbors.size() - 1);
            auto [nx, ny] = neighbors.at(dist(generator));
            RemoveWallBetween(x, y, nx, ny);
            (*this)[nx, ny].visited = true;
            stack.emplace(nx, ny);
        }
    }

private:
    void RemoveWallBetween(const int x, const int y, const int nx, const int ny) {
        if (nx == x + 1) { //E
            (*this)[x, y].wallE = false;
            (*this)[nx, ny].wallW = false;
        }
        else if (nx == x - 1) { //W
            (*this)[x, y].wallW = false;
            (*this)[nx, ny].wallE = false;
        }
        else if (ny == y + 1) { //N
            (*this)[x, y].wallN = false;
            (*this)[nx, ny].wallS = false;
        }
        else if (ny == y - 1) { //N
            (*this)[x, y].wallS = false;
            (*this)[nx, ny].wallN = false;
        }
    }
    int m_Width, m_Height;
    std::vector<MazeCell> m_Cells;

};


struct MazeData {
    std::vector<Engine::Vertex> vertices;
    std::vector<std::uint32_t> indices;
};


static void AddWall(MazeData& data, const float cx, const float cy, const float halfW, const float halfH, const glm::vec4& color) {
    const std::uint32_t base = data.vertices.size();
    data.vertices.emplace_back(glm::vec3(cx - halfW, cy - halfH, 0.0f), color);
    data.vertices.emplace_back(glm::vec3(cx - halfW, cy + halfH, 0.0f), color);
    data.vertices.emplace_back(glm::vec3(cx + halfW, cy + halfH, 0.0f), color);
    data.vertices.emplace_back(glm::vec3(cx + halfW, cy - halfH, 0.0f), color);
    data.indices.insert(data.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
}


static MazeData BuildMaze(const MazeGrid& grid, const float cellSize, const float wallThickness) {
    MazeData data;
    constexpr auto wallColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    constexpr auto entranceColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    constexpr auto exitColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    const auto totalWidth = grid.GetWidth() * cellSize;
    const auto totalHeight = grid.GetHeight() * cellSize;
    const auto originX = -totalWidth * 0.5f;
    const auto originY = -totalHeight * 0.5f;
    for (auto y = 0; y < grid.GetHeight(); ++y) {
        for (auto x = 0; x < grid.GetWidth(); ++x) {
            const auto& cell = grid[x, y];
            const auto cellX = originX + x * cellSize;
            const auto cellY = originY + y * cellSize;
            if (cell.wallN) {
                if (y == grid.GetHeight() - 1 && x == grid.GetWidth() - 1) {
                    AddWall(data, cellX + cellSize * 0.5f, cellY + cellSize, cellSize * 0.5f, wallThickness * 0.5f, exitColor);
                }else {
                    AddWall(data, cellX + cellSize * 0.5f, cellY + cellSize, cellSize * 0.5f, wallThickness * 0.5f, wallColor);
                }
            }
            if (cell.wallE) {
                AddWall(data, cellX + cellSize, cellY + cellSize * 0.5f, wallThickness * 0.5f, cellSize * 0.5f, wallColor);
            }
            if (y == 0 && cell.wallS) {
                if (x == 0) {
                    AddWall(data, cellX + cellSize * 0.5f, cellY, cellSize * 0.5f, wallThickness * 0.5f, entranceColor);
                }else {
                    AddWall(data, cellX + cellSize * 0.5f, cellY, cellSize * 0.5f, wallThickness * 0.5f, wallColor);
                }
            }
            if (x == 0 && cell.wallW) {
                AddWall(data, cellX, cellY + cellSize * 0.5f, wallThickness * 0.5f, cellSize * 0.5f, wallColor);
            }
        }
    }

    return data;
}


class Maze final: public Engine::Scene {
public:

    Maze(): m_CameraController(static_cast<float>(Engine::WindowWidth)/Engine::WindowHeight, 100.0f) {
        MazeGrid grid(10, 10);
        grid.Generate();
        auto [vertices, indices] = BuildMaze(grid, 10.0f, 0.5f);

        const auto vertexBuffer = Engine::VertexBuffer::Create(vertices);
        const auto indexBuffer = Engine::IndexBuffer::Create(indices);
        const auto vertexArray = Engine::VertexArray::Create();
        vertexArray->AddVertexBuffer(vertexBuffer);
        vertexArray->SetIndexBuffer(indexBuffer);
        const auto shader = Engine::Shader::Create("Assets/Shaders/Color.glsl");
        m_Sprite.VA = vertexArray;
        m_Sprite.shader = shader;
    }

    void OnUpdate(const float dt) override {
        Engine::Renderer::Clear();
        Engine::Renderer::Draw(m_Sprite, m_CameraController.GetCamera());
    }

    void OnEvent(Engine::Event &event) override {
        m_CameraController.OnEvent(event);
    }

private:

    Engine::Sprite m_Sprite;
    Engine::OrthographicCameraController m_CameraController;
};

class AI_Escape: public Engine::Application {
public:
    AI_Escape() {
        SetScene(std::make_unique<Maze>());
    }
};


std::unique_ptr<Engine::Application> Engine::CreateApplication() {
    return std::make_unique<AI_Escape>();
}
