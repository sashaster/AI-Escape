#pragma once
#include <Engine/Renderer/VertexArray.hpp>
#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/OrthographicCamera.hpp>


namespace Engine {

    struct Sprite {
        std::shared_ptr<VertexArray> VA;
        std::shared_ptr<Shader> shader;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    class Renderer {
    public:
        static void Draw(const Sprite& sprite, const OrthographicCamera& camera);
        static void Clear();
        static void SetViewport(int width, int height);
    };
}