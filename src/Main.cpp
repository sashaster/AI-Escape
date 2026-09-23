#include <cstdint>


#include <Engine/Entry_Point.hpp>
#include <Engine/Application.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/OrthographicCameraController.hpp>


class Maze final: public Engine::Scene {
public:

    Maze(): m_CameraController(1280.0f/720.0f) {
        constexpr std::array vertices = {
            -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f, 1.0f
        };

        constexpr std::array<std::uint32_t, 6> indices = {
            0, 1, 2,
            2, 3, 0
        };

        const auto vertexBuffer = Engine::VertexBuffer::Create(vertices.data(), sizeof(vertices));
        vertexBuffer->SetLayout({
            {Engine::VertexAttribute::AttributeType::Float3, "a_Position"},
            {Engine::VertexAttribute::AttributeType::Float4, "a_Color"}
        });
        const auto indexBuffer = Engine::IndexBuffer::Create(indices.data(), indices.size());
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
