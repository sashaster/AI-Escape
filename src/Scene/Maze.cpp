#include <stack>
#include <utility>

#include <Scene/Maze.hpp>
#include <Scene/Agent.hpp>

#include <Engine/Core/Random.hpp>

namespace Scene {

    void MazeGrid::Generate() {
        Reset();
        constexpr int startX = 0, startY = 0;
        std::stack<std::pair<int, int> > stack;
        (*this)[startX, startY].visited = true;
        stack.emplace(startX, startY);
        while (!stack.empty()) {
            auto [x, y] = stack.top();
            std::array<std::pair<int, int>, 4 > neighbors;
            auto i = 0;
            if (y < m_Height - 1 && !(*this)[x, y + 1].visited) {
                //N
                neighbors[i++] = {x, y + 1};
            }
            if (y > 0 && !(*this)[x, y - 1].visited) {
                //S
                neighbors[i++] = {x, y - 1};
            }
            if (x > 0 && !(*this)[x - 1, y].visited) {
                //W
                neighbors[i++] = {x - 1, y};
            }
            if (x < m_Width - 1 && !(*this)[x + 1, y].visited) {
                //E
                neighbors[i++] = {x + 1, y};
            }
            if (i == 0) {
                stack.pop();
                continue;
            }

            const auto max = i - 1;

            auto [nx, ny] = neighbors.at(Engine::Random::Uint(0, max));
            RemoveWallBetween(x, y, nx, ny);
            (*this)[nx, ny].visited = true;
            stack.emplace(nx, ny);
        }
    }

    void MazeGrid::Reset() {
        for (auto x = 0; x < m_Width; ++x) {
            for (auto y = 0; y < m_Height; ++y) {
                auto& cell = (*this)[x, y];
                cell.wallN = cell.wallS = cell.wallE = cell.wallW = true;
                cell.visited = false;
            }
        }
    }

    void MazeGrid::RemoveWallBetween(const std::uint32_t x, const std::uint32_t y, const std::uint32_t nx, const std::uint32_t ny) {
        if (nx == x + 1) {
            //E
            (*this)[x, y].wallE = false;
            (*this)[nx, ny].wallW = false;
        } else if (nx == x - 1) {
            //W
            (*this)[x, y].wallW = false;
            (*this)[nx, ny].wallE = false;
        } else if (ny == y + 1) {
            //N
            (*this)[x, y].wallN = false;
            (*this)[nx, ny].wallS = false;
        } else if (ny == y - 1) {
            //S
            (*this)[x, y].wallS = false;
            (*this)[nx, ny].wallN = false;
        }
    }

    static Engine::Physics::AABB Box(const float cx, const float cy, const float halfW, const float halfH) {
        return {.min = {cx - halfW, cy - halfH}, .max = {cx + halfW, cy + halfH}};
    }

    void Maze::BuildColliders() {
        for (auto y = 0; y < m_Grid.GetHeight(); ++y) {
            for (auto x = 0; x < m_Grid.GetWidth(); ++x) {
                const auto &cell = m_Grid[x, y];
                const auto cellPos = Cell({x, y});
                if (cell.wallN) {
                    m_Walls.emplace_back(Box(cellPos.x + m_CellSize * 0.5f,  cellPos.y + m_CellSize,  m_CellSize * 0.5f, m_WallThickness * 0.5f));
                }
                if (m_LastCell.x == x && m_LastCell.y == y) {
                    const float topY = cellPos.y + m_CellSize;
                    m_Exit = {.min = {cellPos.x, topY}, .max = {cellPos.x + m_CellSize, topY + m_CellSize}};
                }
                if (cell.wallE) {
                    m_Walls.emplace_back(Box(cellPos.x + m_CellSize, cellPos.y + m_CellSize * 0.5f, m_WallThickness * 0.5f, m_CellSize * 0.5f));
                }
                if (y == 0 && cell.wallS) {
                    m_Walls.emplace_back(Box(cellPos.x + m_CellSize * 0.5f, cellPos.y, m_CellSize * 0.5f, m_WallThickness * 0.5f));
                }
                if (x == 0 && cell.wallW) {
                    m_Walls.emplace_back(Box(cellPos.x, cellPos.y + m_CellSize * 0.5f, m_WallThickness * 0.5f, m_CellSize * 0.5f));
                }
            }
        }
    }

    Maze::Maze(const std::uint32_t gridWidth, const std::uint32_t gridHeight, const float cellSize, const float wallThickness, const std::uint32_t seed):
                m_Grid(gridWidth, gridHeight),
                m_Width(static_cast<float>(m_Grid.GetWidth()) * cellSize),
                m_Height(static_cast<float>(m_Grid.GetHeight()) * cellSize),
                m_CellSize(cellSize),
                m_WallThickness(wallThickness),
                m_Seed(seed)
    {
        m_LastCell.x = m_Grid.GetWidth() - 1;
        m_LastCell.y = m_Grid.GetHeight() - 1;
        Generate(m_Seed);
    }

    void Maze::Generate(const std::uint32_t seed) {
        Engine::Random::Seed(seed);
        m_Seed = seed;
        m_Grid.Generate();
        m_Grid[m_LastCell.x, m_LastCell.y].wallN = false;
        m_Walls.clear();
        BuildColliders();
    }

}
