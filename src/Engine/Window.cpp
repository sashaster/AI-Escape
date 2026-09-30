#include <glad/gl.h>

#include <Engine/Window.hpp>
#include <Engine/Core.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Events/ApplicationEvent.hpp>


namespace Engine {

    Window::Window(const std::string_view title, const int width, const int height) {
        m_Data.title = title;
        m_Data.width = width;
        m_Data.height = height;
        Init();
    }

    void Window::Init() {
        INFO("Creating window...{} ({}x{})", m_Data.title, m_Data.width, m_Data.height);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        m_Handle.reset(glfwCreateWindow(m_Data.width, m_Data.height, m_Data.title.c_str(), nullptr, nullptr));
        glfwMakeContextCurrent(m_Handle.get());
        DEBUG("Loading OpenGL...");
        ASSERT(gladLoadGL(glfwGetProcAddress), "Failed to load OpenGL");
        DEBUG("OpenGL initialized");
        INFO("OpenGL: {0} {1} {2}",
            reinterpret_cast<const char*>(glGetString(GL_VENDOR)),
            reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
            reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        glfwSetWindowUserPointer(m_Handle.get(), &m_Data);
        glfwSwapInterval(1);

        glfwSetWindowCloseCallback(m_Handle.get(), [](GLFWwindow* window) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            ASSERT(data, "Window data is null");
            WindowCloseEvent event;
            data->eventCallback(event);
        });

        glfwSetFramebufferSizeCallback(m_Handle.get(), [](GLFWwindow* window, int width, int height) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            ASSERT(data, "Window data is null");

            data->width = width;
            data->height = height;
            Renderer::SetViewport(width, height);
            WindowResizeEvent event(width, height);
            data->eventCallback(event);
        });
    }

    void Window::OnUpdate() {
        glfwSwapBuffers(m_Handle.get());
        glfwPollEvents();
    }
}
