#pragma once
#include <random>
#include <utility>

#include <Engine/Core/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Core/Physics.hpp>
#include <Scene/Agent.hpp>


namespace Scene {

    struct MazeCell {
        bool visited = false;
        bool wallN = true, wallS = true, wallE = true, wallW = true;
    };

    class MazeGrid {
    public:
        MazeGrid(const int width, const int height) : m_Width(width), m_Height(height), m_Cells(m_Width * m_Height) {
            ASSERT(m_Width > 0 && m_Height > 0, "Invalid grid dimensions")
        }

        const MazeCell &operator[](const int x, const int y) const {
            return m_Cells.at(y * m_Width + x);
        }

        MazeCell &operator[](const int x, const int y) {
            return m_Cells.at(y * m_Width + x);
        }

        [[nodiscard]] int GetWidth() const {
            return m_Width;
        }

        [[nodiscard]] int GetHeight() const {
            return m_Height;
        }

        void Generate(std::uint32_t seed);

    private:
        void RemoveWallBetween(int x, int y, int nx, int ny);
        void Reset();
        int m_Width, m_Height;
        std::vector<MazeCell> m_Cells;
    };

    class Maze {
    public:
        [[nodiscard]] glm::vec2 operator[](const int x, const int y) const {
            return GridToWorld(x, y) + glm::vec2(m_CellSize * 0.5f);
        }

        [[nodiscard]] glm::vec2 Origin() const {
            return {-m_Width * 0.5f, -m_Height * 0.5f};
        }

        [[nodiscard]] const MazeGrid& GetGrid() const{
            return m_Grid;
        }

        [[nodiscard]] float GetCellSize() const {
            return m_CellSize;
        }

        [[nodiscard]] glm::vec2 Size() const {
            return {m_Width, m_Height};
        }

        [[nodiscard]] float GetWallThickness() const {
            return m_WallThickness;
        }

        [[nodiscard]] std::uint32_t GetSeed() const {
            return m_Seed;
        }

        [[nodiscard]] glm::ivec2 GetExitCell() const {
            return m_ExitCell;
        }
        Maze(int gridWidth, int gridHeight, float cellSize, float wallThickness, glm::ivec2 exitCell, std::uint32_t seed = std::random_device{}());

        void Generate(std::uint32_t seed);
        [[nodiscard]] glm::vec2 GridToWorld(const int x, const int y) const {
            return Origin() + glm::vec2(x, y) * m_CellSize;
        }
        [[nodiscard]] std::span<const Engine::Physics::AABB> GetWalls() const {
            return m_Walls;
        }
        [[nodiscard]] Engine::Physics::AABB GetExit() const {
            return m_Exit;
        }
        [[nodiscard]] float GetWidth() const {
            return m_Width;
        }
        [[nodiscard]] float GetHeight() const {
            return m_Height;
        }

    private:
        void BuildColliders();
        std::vector<Engine::Physics::AABB> m_Walls;
        Engine::Physics::AABB m_Exit;
        glm::ivec2 m_ExitCell;
        MazeGrid m_Grid;
        float m_Width;
        float m_Height;
        float m_CellSize;
        float m_WallThickness;
        std::uint32_t m_Seed;
    };


}
