#include <string_view>
#include <fstream>
#include <sstream>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>


#include <Engine/Renderer/Shader.hpp>


namespace Engine {

    GLenum Shader::GLtype(const std::string_view type) {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment")
            return GL_FRAGMENT_SHADER;
        ASSERT(false, "Unknown shader type {}", type);
        return GL_NONE;
    }

    void Shader::Bind() const {
        glUseProgram(m_ID);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    std::shared_ptr<Shader> Shader::Create(const std::filesystem::path& path) {
        return std::shared_ptr<Shader>(new Shader(path));
    }

    Shader::~Shader() {
        glDeleteProgram(m_ID);
    }

    std::unordered_map<GLenum, std::string> Shader::Parse(const std::filesystem::path& path) {
        std::unordered_map<GLenum, std::string> shaders;
        std::ifstream file(path);
        if (!file) {
            ASSERT(false, "Could not open file {}", path.string());
        }
        std::stringstream ss;
        ss << file.rdbuf();
        const auto source = ss.str();
        constexpr std::string_view token = "#shader";
        auto pos = source.find(token);
        while (pos != std::string::npos) {
            auto eol = source.find('\n', pos);
            auto typeStart = pos + token.length() + 1;
            auto type = source.substr(typeStart, eol - typeStart);
            auto newLine = source.find_first_not_of('\n', eol);
            pos = source.find(token, newLine);
            shaders[GLtype(type)] = source.substr(newLine, pos - newLine);
        }
        ASSERT(shaders.size() == 2, "Failed to parse shader {}", path.string())
        return shaders;
    }

    Shader::Shader(const std::filesystem::path &path){
        Link(Parse(path));
    }

    std::string_view Shader::ShaderTypeName(const GLenum type) {
        switch (type) {
            case GL_VERTEX_SHADER:
                return "Vertex";
            case GL_FRAGMENT_SHADER:
                return "Fragment";
            default:
                break;
        }
        ASSERT(false, "Unknown shader type {}", type);
        return "";
    }

    void Shader::Link(const std::unordered_map<GLenum, std::string>& sources){
        TRACE("Linking shader program...");
        m_ID = glCreateProgram();
        std::array<std::uint32_t, 2> shaderIDs{};
        int i = 0;
        for (const auto& [type, source] : sources) {
            const auto shader = Compile(source, type);
            glAttachShader(m_ID, shader);
            shaderIDs[i++] = shader;
        }
        glLinkProgram(m_ID);
        for (const auto id : shaderIDs) {
            glDetachShader(m_ID, id);
            glDeleteShader(id);
        }
        Validate(m_ID, GL_NONE);
    }

    void Shader::Validate(const std::uint32_t id, const GLenum type) const {
        GLint success;
        GLint length;
        if (type != GL_NONE) {
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
                std::string message(length, '\0');
                glGetShaderInfoLog(id, length, &length, message.data());
                ERROR("Failed to compile {} shader {}",
                    ShaderTypeName(type),
                    message);
            }
        }else {
            glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramiv(m_ID, GL_INFO_LOG_LENGTH, &length);
                std::string message(length, '\0');
                glGetProgramInfoLog(m_ID, length, &length, message.data());
                ERROR("Failed to link shader program: {}",
                    message);
            }
        }
    }

    std::uint32_t Shader::Compile(const std::string_view source, const GLenum type) const {
        TRACE("Compiling {} shader...", ShaderTypeName(type));
        const auto shader = glCreateShader(type);
        const auto src = source.data();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        Validate(shader, type);
        return shader;
    }

    void Shader::SetFloat4(const std::string_view name, const glm::vec4 values) noexcept {
        glUniform4f(glGetUniformLocation(m_ID, name.data()), values.r, values.g, values.b, values.a);
    }

    void Shader::SetMat4(const std::string_view name, const glm::mat4& values) noexcept {
        glUniformMatrix4fv(glGetUniformLocation(m_ID, name.data()), 1, GL_FALSE, glm::value_ptr(values));
    }
}
