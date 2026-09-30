#include <stack>
#include <utility>


#include <Scene/Maze.hpp>

namespace Scene {

    void MazeGrid::Generate(const std::uint32_t seed) {
        std::mt19937 generator(seed);
        constexpr int startX = 0, startY = 0;
        std::stack<std::pair<int, int> > stack;
        (*this)[startX, startY].visited = true;
        stack.emplace(startX, startY);
        while (!stack.empty()) {
            auto [x, y] = stack.top();
            std::vector<std::pair<int, int> > neighbors;
            if (y < m_Height - 1 && !(*this)[x, y + 1].visited) {
                //N
                neighbors.emplace_back(x, y + 1);
            }
            if (y > 0 && !(*this)[x, y - 1].visited) {
                //S
                neighbors.emplace_back(x, y - 1);
            }
            if (x > 0 && !(*this)[x - 1, y].visited) {
                //W
                neighbors.emplace_back(x - 1, y);
            }
            if (x < m_Width - 1 && !(*this)[x + 1, y].visited) {
                //E
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

    void MazeGrid::RemoveWallBetween(const int x, const int y, const int nx, const int ny) {
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
            //N
            (*this)[x, y].wallS = false;
            (*this)[nx, ny].wallN = false;
        }
    }

    void Maze::AddWall(const float cx, const float cy, const float halfW, const float halfH,
                        const glm::vec4 &color, const bool solid) {
        const std::uint32_t base = m_Data.vertices.size();
        m_Data.vertices.emplace_back(glm::vec3(cx - halfW, cy - halfH, 0.0f), color);
        m_Data.vertices.emplace_back(glm::vec3(cx - halfW, cy + halfH, 0.0f), color);
        m_Data.vertices.emplace_back(glm::vec3(cx + halfW, cy + halfH, 0.0f), color);
        m_Data.vertices.emplace_back(glm::vec3(cx + halfW, cy - halfH, 0.0f), color);
        m_Data.indices.insert(m_Data.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
        if (solid) {
            m_Data.walls.push_back({.min = {cx - halfW, cy - halfH}, .max = {cx + halfW, cy + halfH}});
        }
    }

    Maze::Maze(const MazeGrid& grid, const float cellSize, const float wallThickness): m_Grid(grid){
        m_Data.width = m_Grid.GetWidth() * cellSize;
        m_Data.height = m_Grid.GetHeight() * cellSize;
        m_Data.cellSize = cellSize;
        m_Grid.Generate(0);
        BuildMaze(wallThickness);
    }

    void Maze::BuildMaze(const float wallThickness) {
        constexpr auto wallColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        constexpr auto exitColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        constexpr auto floorColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        for (auto y = 0; y < m_Grid.GetHeight(); ++y) {
            for (auto x = 0; x < m_Grid.GetWidth(); ++x) {
                const auto &cell = m_Grid[x, y];
                const auto [cellX, cellY] = ToWorldSpace(x, y);
                AddWall(cellX + m_Data.cellSize * 0.5f, cellY + m_Data.cellSize * 0.5f, m_Data.cellSize * 0.5f, m_Data.cellSize * 0.5f, floorColor, false);
                if (cell.wallN) {
                    const auto cx = cellX + m_Data.cellSize * 0.5f;
                    const auto cy = cellY + m_Data.cellSize;
                    const auto hw = m_Data.cellSize * 0.5f;
                    const auto hh = wallThickness * 0.5f;
                    if (y == m_Grid.GetHeight() - 1 && x == m_Grid.GetWidth() - 1) {
                        AddWall(cx, cy, hw, hh, exitColor, false);
                        m_Data.exit = {.min = {cx - hw, cy - hh}, .max = {cx + hw, cy + hh}};
                    } else {
                        AddWall(cx, cy, hw, hh, wallColor);
                    }
                }
                if (cell.wallE) {
                    AddWall(cellX + m_Data.cellSize, cellY + m_Data.cellSize * 0.5f, wallThickness * 0.5f, m_Data.cellSize* 0.5f,
                            wallColor);
                }
                if (y == 0 && cell.wallS) {
                    AddWall(cellX + m_Data.cellSize * 0.5f, cellY, m_Data.cellSize * 0.5f, wallThickness * 0.5f, wallColor);
                }
                if (x == 0 && cell.wallW) {
                    AddWall(cellX, cellY + m_Data.cellSize * 0.5f, wallThickness * 0.5f, m_Data.cellSize * 0.5f, wallColor);
                }
            }
        }

    }


}