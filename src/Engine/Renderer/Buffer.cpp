#include <glad/gl.h>


#include <Engine/Renderer/Buffer.hpp>


namespace Engine {
    VertexBuffer::VertexBuffer(const void* vertices, const std::uint32_t size) {
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* vertices, const std::uint32_t size) {
        return std::shared_ptr<VertexBuffer>(new VertexBuffer(vertices, size));
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }

    void VertexBuffer::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
    }

    void VertexBuffer::Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    IndexBuffer::IndexBuffer(const std::uint32_t* indices, const std::uint32_t count): m_Count(count) {
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(std::uint32_t), indices, GL_STATIC_DRAW);
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Create(const std::uint32_t* indices, const std::uint32_t count) {
        return std::shared_ptr<IndexBuffer>(new IndexBuffer(indices, count));
    }

    void IndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
    }

    void IndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }


    std::uint32_t VertexAttribute::GetCount() const {
        switch (type) {
            case AttributeType::Float:
                return 1;
            case AttributeType::Float2:
                return 2;
            case AttributeType::Float3:
                return 3;
            case AttributeType::Float4:
                return 4;
            case AttributeType::None:
                break;
        }
        ASSERT(false, "Unknown attribute type");
        return 0;
    }

    GLenum VertexAttribute::GLType(const AttributeType type){
        switch (type) {
            case AttributeType::Float:
            case AttributeType::Float2:
            case AttributeType::Float3:
            case AttributeType::Float4:
                return GL_FLOAT;
            case AttributeType::None:
                break;
        }
        ASSERT(false, "Unknown attribute type");
        return 0;
    }

    std::uint32_t VertexAttribute::GetSize() const{
        switch (type) {
            case AttributeType::Float:
                return 4;
            case AttributeType::Float2:
                return 4 * 2;
            case AttributeType::Float3:
                return 4 * 3;
            case AttributeType::Float4:
                return 4 * 4;
            case AttributeType::None:
                break;
        }
        ASSERT(false, "Unknown element type");
        return 0;
    }

    VertexLayout::VertexLayout(const std::initializer_list<VertexAttribute>& attributes): m_Attributes(attributes) {
        CalculateStrideAndOffset();
    }

    void VertexLayout::CalculateStrideAndOffset() {
        for (auto& attribute : m_Attributes) {
            attribute.offset = m_Stride;
            m_Stride += attribute.GetSize();
        }
    }

}
