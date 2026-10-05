#include <Engine/Renderer/Renderer.hpp>


namespace Engine {

    void Renderer::Clear() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::Draw(const Sprite &sprite, const OrthographicCamera& camera, const bool lines) {
        sprite.VA->Bind();
        sprite.shader->Bind();
        sprite.shader->SetMat4("u_ViewProjection", camera.GetViewProjection());
        sprite.shader->SetMat4("u_Model", sprite.transform);
        if (!lines) {
            glDrawElements(GL_TRIANGLES, sprite.VA->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
        }
        else {
            glDrawArrays(GL_LINES, 0, sprite.VA->GetBuffers().front()->GetCount());
        }
    }

    void Renderer::SetViewport(const int width, const int height) {
        glViewport(0, 0, width, height);
    }

}
