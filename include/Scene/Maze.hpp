#pragma once
#include <random>
#include <utility>

#include <Engine/Core/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Core/Physics.hpp>


namespace Scene {

    struct MazeCell {
        bool visited = false;
        bool wallN = true, wallS = true, wallE = true, wallW = true;
    };


    class MazeGrid {
    public:
        MazeGrid(const int width, const int height) : m_Width(width), m_Height(height), m_Cells(width * height) {
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
        struct MazeData {
            std::vector<Engine::Vertex> vertices;
            std::vector<std::uint32_t> indices;
            float width;
            float height;
            float cellSize;
            float wallThickness;
            std::vector<Engine::Physics::AABB> walls;
            Engine::Physics::AABB exit;

        } m_Data;

        [[nodiscard]] glm::vec2 operator[](const int x, const int y) const {
            const auto [cellX, cellY] = ToWorldSpace(x, y);
            return {cellX + m_Data.cellSize * 0.5f, cellY + m_Data.cellSize * 0.5f};
        }

        [[nodiscard]] std::pair<float, float> GetOrigin() const {
            return {-m_Data.width * 0.5f, -m_Data.height * 0.5f};
        }

        Maze(const MazeGrid& grid, float cellSize, float wallThickness);
        void BuildMaze();
        [[nodiscard]] const MazeData& GetData() const {
            return m_Data;
        }

        void Regenerate(std::uint32_t seed);

    private:
        void AddWall(float cx, float cy, float halfW, float halfH, const glm::vec4 &color, bool solid=true);
        [[nodiscard]] std::pair<float, float> ToWorldSpace(const int x, const int y) const {
            const auto [originX, originY] = GetOrigin();
            return {originX + x * m_Data.cellSize, originY + y * m_Data.cellSize};
        }
        MazeGrid m_Grid;
    };


}
