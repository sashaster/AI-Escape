#pragma once
#include <Engine/Renderer/Renderer.hpp>
#include <Scene/Maze.hpp>


namespace Scene {

    class SceneRenderer {
    public:
        explicit SceneRenderer(const Maze& maze);
        void Draw();
        void Submit(const Agent& agent, const Maze& maze, const Engine::OrthographicCamera& camera, std::span<const Engine::Physics::Ray> rays, bool sensors);

    private:
        void Init();
        static void AddWall(float cx, float cy, float halfW, float halfH, glm::vec4 color);
        static void BuildMaze(const Maze& maze);
        Engine::Sprite m_Maze;
        Engine::Sprite m_Agent;
        Engine::Sprite m_Sensor;
        bool m_Sensors = false;
        glm::mat4 m_ViewProjection{0.0f};
        std::uint32_t m_MazeSeed;
    };
}