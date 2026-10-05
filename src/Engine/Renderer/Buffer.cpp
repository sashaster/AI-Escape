#include <glad/gl.h>


#include <Engine/Renderer/Buffer.hpp>


namespace Engine {
    VertexBuffer::VertexBuffer(const std::span<const Vertex> vertices) {
        ASSERT(!vertices.empty(), "Vertex buffer cannot be empty")
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
        m_Layout = Vertex::GetLayout();
    }

    VertexBuffer::VertexBuffer(const std::uint32_t count): m_Count(count) {
        ASSERT(count > 0, "VertexBuffer capacity must be greater than 0")
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ARRAY_BUFFER, m_Count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
        m_Layout = Vertex::GetLayout();
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(const std::span<const Vertex> vertices) {
        return std::shared_ptr<VertexBuffer>(new VertexBuffer(vertices));
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(const std::uint32_t count) {
        return std::shared_ptr<VertexBuffer>(new VertexBuffer(count));
    }

    void VertexBuffer::SetData(const std::span<const Vertex> vertices) {
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size_bytes(), vertices.data());
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

    IndexBuffer::IndexBuffer(const std::span<const std::uint32_t> indices): m_Count(indices.size()) {
        ASSERT(!indices.empty(), "Index buffer cannot be empty")
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Create(const std::span<const std::uint32_t> indices) {
        return std::shared_ptr<IndexBuffer>(new IndexBuffer(indices));
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