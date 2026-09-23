#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glad/gl.h>


#include <Engine/Core.hpp>


namespace Engine {

    struct VertexAttribute final {

        enum class AttributeType: std::uint8_t {
            None,
            Float,
            Float2,
            Float3,
            Float4
        };

        VertexAttribute(const AttributeType type, const std::string_view name, const bool normalized = false): type(type), normalized(normalized){}
        [[nodiscard]] static GLenum GLType(AttributeType type);
        [[nodiscard]] std::uint32_t GetSize() const;
        [[nodiscard]] std::uint32_t GetCount() const;

        std::uint32_t offset = 0;
        AttributeType type;
        bool normalized;
    };

    class VertexLayout final {
    public:
        VertexLayout() = default;
        VertexLayout(const std::initializer_list<VertexAttribute>& attributes);

        [[nodiscard]] auto begin() const noexcept {
            return m_Attributes.begin();
        }

        [[nodiscard]] auto end() const noexcept {
            return m_Attributes.end();
        }

        [[nodiscard]] std::uint32_t GetStride() const{
            return m_Stride;
        }

    private:
        void CalculateStrideAndOffset();

        std::uint32_t m_Stride = 0;
        std::vector<VertexAttribute> m_Attributes;
    };

    class VertexBuffer {
    public:
        ~VertexBuffer();
        void Bind() const;
        void Unbind() const;
        void SetLayout(const VertexLayout& layout) {
            m_Layout = layout;
        }
        [[nodiscard]] const VertexLayout& GetLayout() const {
            return m_Layout;
        }
        static std::shared_ptr<VertexBuffer> Create(const void* vertices, std::uint32_t size);

    private:
        explicit VertexBuffer(const void* vertices, std::uint32_t size);
        VertexLayout m_Layout;
        RendererID m_ID;
    };

    class IndexBuffer {
    public:
        static std::shared_ptr<IndexBuffer> Create(const std::uint32_t* indices, std::uint32_t count);
        [[nodiscard]] std::uint32_t GetCount() const {
            return m_Count;
        }
        void Bind() const;
        void Unbind() const;
        ~IndexBuffer();

    private:
        explicit IndexBuffer(const std::uint32_t* indices, std::uint32_t count);
        RendererID m_ID;
        std::uint32_t m_Count;
    };

}