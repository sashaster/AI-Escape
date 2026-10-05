#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <span>


#include <Engine/Core/Core.hpp>


namespace Engine {

    class VertexLayout;

    struct VertexAttribute final {

        enum class AttributeType: std::uint8_t {
            None,
            Float,
            Float2,
            Float3,
            Float4
        };

        VertexAttribute(const AttributeType type, const std::string_view name, const bool normalized = false): type(type), normalized(normalized), name(name){}
        [[nodiscard]] static GLenum GLType(AttributeType type);
        [[nodiscard]] std::uint32_t GetSize() const;
        [[nodiscard]] std::uint32_t GetCount() const;

        std::uint32_t offset = 0;
        AttributeType type;
        bool normalized;
        std::string name;
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

    struct Vertex {
        glm::vec3 position;
        glm::vec4 color;

        [[nodiscard]] static VertexLayout GetLayout(){
            return {
                {VertexAttribute::AttributeType::Float3, "position"},
                {VertexAttribute::AttributeType::Float4, "color"}
            };
        }
    };

    class VertexBuffer final {
    public:
        ~VertexBuffer();
        void Bind() const;
        void Unbind() const;
        [[nodiscard]] const VertexLayout& GetLayout() const {
            return m_Layout;
        }
        [[nodiscard]] std::uint32_t GetCount() const {
            return m_Count;
        }

        void SetData(std::span<const Vertex> vertices);

        static std::shared_ptr<VertexBuffer> Create(std::span<const Vertex> vertices);
        static std::shared_ptr<VertexBuffer> Create(std::uint32_t count);

    private:
        explicit VertexBuffer(std::span<const Vertex> vertices);
        explicit VertexBuffer(std::uint32_t count);
        VertexLayout m_Layout;
        RendererID m_ID;
        std::uint32_t m_Count;
    };

    class IndexBuffer final {
    public:
        static std::shared_ptr<IndexBuffer> Create(std::span<const std::uint32_t> indices);
        [[nodiscard]] std::uint32_t GetCount() const {
            return m_Count;
        }
        void Bind() const;
        void Unbind() const;
        ~IndexBuffer();

    private:
        explicit IndexBuffer(std::span<const std::uint32_t> indices);
        RendererID m_ID;
        std::uint32_t m_Count;
    };



}