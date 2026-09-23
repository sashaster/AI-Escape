#include <glad/gl.h>


#include <Engine/Renderer/VertexArray.hpp>


namespace Engine {

    VertexArray::VertexArray(): m_AttributeIndex(0){
        glGenVertexArrays(1, &m_ID);
    }

    void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
        ASSERT(indexBuffer, "Index buffer is null");
        glBindVertexArray(m_ID);
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_ID);
    }

    void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
        glBindVertexArray(m_ID);
        vertexBuffer->Bind();
        for (auto& attribute: vertexBuffer->GetLayout()) {
            glVertexAttribPointer(m_AttributeIndex,
                attribute.GetCount(),
                VertexAttribute::GLType(attribute.type),
                attribute.normalized ? GL_TRUE : GL_FALSE,
                vertexBuffer->GetLayout().GetStride(),
                reinterpret_cast<void*>(attribute.offset));
            glEnableVertexAttribArray(m_AttributeIndex);
            ++m_AttributeIndex;
            m_VertexBuffers.push_back(vertexBuffer);
        }
    }

    void VertexArray::Bind() const {
        glBindVertexArray(m_ID);
    }

    void VertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    std::shared_ptr<VertexArray> VertexArray::Create() {
        return std::shared_ptr<VertexArray>(new VertexArray());
    }

}