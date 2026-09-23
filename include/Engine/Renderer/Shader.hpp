#pragma once
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <string_view>
#include <glm/glm.hpp>


#include <Engine/Core.hpp>


namespace Engine {

    class Shader {
    public:
        static std::shared_ptr<Shader> Create(const std::filesystem::path& path);
        void Bind() const;
        void Unbind() const;
        ~Shader();
        void SetFloat4(std::string_view name, const glm::vec4& values) noexcept;
        void SetMat4(std::string_view name, const glm::mat4& values) noexcept;
    private:
        void Link(const std::unordered_map<GLenum, std::string> &sources);
        void Validate(std::uint32_t id, GLenum  type) const;
        [[nodiscard]] std::uint32_t Compile(std::string_view source, GLenum type) const;
        static GLenum GLtype(std::string_view type);
        static std::string_view ShaderTypeName(GLenum type);
        static std::unordered_map<GLenum, std::string> Parse(const std::filesystem::path& path);
        explicit Shader(const std::filesystem::path& path);
        RendererID m_ID;
    };
}