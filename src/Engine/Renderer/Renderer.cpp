#include <Engine/Renderer/Renderer.hpp>


namespace Engine {

    void Renderer::Clear() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::Draw(const Sprite &sprite, const OrthographicCamera& camera) {
        sprite.VA->Bind();
        sprite.shader->Bind();
        sprite.shader->SetMat4("u_ViewProjection", camera.GetViewProjection());
        sprite.shader->SetMat4("u_Model", sprite.transform);
        glDrawElements(GL_TRIANGLES, sprite.VA->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
    }

    void Renderer::SetViewport(const int width, const int height) {
        glViewport(0, 0, width, height);
    }

}
