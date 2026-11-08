#pragma once

#include <Engine/Core/Core.hpp>
#include <Engine/Core/Physics.hpp>
#include <random>


namespace Scene {

    struct MazeCell {
        bool visited = false;
        bool wallN = true, wallS = true, wallE = true, wallW = true;
    };

    class MazeGrid {
    public:
        MazeGrid(const std::uint32_t width, const std::uint32_t height) : m_Width(width), m_Height(height), m_Cells(m_Width * m_Height) {
            VERIFY(m_Width > 0 && m_Height > 0, "Invalid grid dimensions");
        }

        const MazeCell &operator[](const std::uint32_t x, const std::uint32_t y) const {
            return m_Cells.at(y * m_Width + x);
        }

        [[nodiscard]] bool InBounds(const std::uint32_t x, const std::uint32_t y) const{
            return x < m_Width && y < m_Height;
        }

        [[nodiscard]] bool InBounds(const glm::ivec2 c) const {
            return c.x >= 0 && c.y >= 0 && InBounds(static_cast<std::uint32_t>(c.x), static_cast<std::uint32_t>(c.y));
        }

        MazeCell &operator[](const std::uint32_t x, const std::uint32_t y) {
            return m_Cells.at(y * m_Width + x);
        }

        [[nodiscard]] std::uint32_t GetWidth() const {
            return m_Width;
        }

        [[nodiscard]] std::uint32_t GetHeight() const {
            return m_Height;
        }

        void Generate();

    private:
        void RemoveWallBetween(std::uint32_t x, std::uint32_t y, std::uint32_t nx, std::uint32_t ny);
        void Reset();
        std::uint32_t m_Width, m_Height;
        std::vector<MazeCell> m_Cells;
    };

    class Maze {
    public:
        [[nodiscard]] glm::vec2 CellCenter(const glm::uvec2 local) const {
            return Cell(local) + glm::vec2(m_CellSize * 0.5f);
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

        [[nodiscard]] glm::uvec2 GetExitCell() const {
            return m_ExitCell;
        }
        Maze(std::uint32_t gridWidth, std::uint32_t gridHeight, float cellSize, float wallThickness, std::uint32_t seed = std::random_device{}());
        void Generate(std::uint32_t seed);
        [[nodiscard]] glm::vec2 Cell(const glm::uvec2 local) const {
            return Origin() + glm::vec2(local.x, local.y) * m_CellSize;
        }
        [[nodiscard]] glm::ivec2 CellAt(const glm::vec2 world) const {
            return glm::ivec2(glm::floor((world - Origin()) / m_CellSize));
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
        [[nodiscard]] std::size_t VisitedIndex(const std::uint32_t x, const std::uint32_t y) const {
            return y * m_Grid.GetWidth() + x;
        }
        void AddExitCell();
        std::vector<Engine::Physics::AABB> m_Walls;
        Engine::Physics::AABB m_Exit;
        glm::uvec2 m_ExitCell;
        MazeGrid m_Grid;
        float m_Width;
        float m_Height;
        float m_CellSize;
        float m_WallThickness;
        std::uint32_t m_Seed;
    };


}
