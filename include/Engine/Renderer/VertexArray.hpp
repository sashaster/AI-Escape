#pragma once
#include <vector>


#include <Engine/Core/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>


namespace Engine {

    class VertexArray {
    public:
        void Bind() const;
        void Unbind() const;
        static std::shared_ptr<VertexArray> Create();
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);
        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
        [[nodiscard]] const IndexBuffer& GetIndexBuffer() const {
            return *m_IndexBuffer;
        }

        ~VertexArray();
    private:
        VertexArray();
        RendererID m_ID;
        std::uint32_t m_AttributeIndex;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
        std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    };
}