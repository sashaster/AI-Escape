#include <glad/gl.h>

#include <Engine/Core/Window.hpp>
#include <Engine/Core/Core.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Events/ApplicationEvent.hpp>
#include <Engine/Events/KeyEvent.hpp>
#include <Engine/Events/MouseEvent.hpp>


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
        VERIFY(gladLoadGL(glfwGetProcAddress), "Failed to load OpenGL");
        DEBUG("OpenGL initialized");
        INFO("OpenGL: {0} {1} {2}",
            reinterpret_cast<const char*>(glGetString(GL_VENDOR)),
            reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
            reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        glfwSetWindowUserPointer(m_Handle.get(), &m_Data);
        glfwSwapInterval(1);

        glfwSetWindowCloseCallback(m_Handle.get(), [](GLFWwindow* window) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");
            WindowCloseEvent event;
            data->eventCallback(event);
        });

        glfwSetFramebufferSizeCallback(m_Handle.get(), [](GLFWwindow* window, const int width, const int height) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");

            data->framebufferWidth = width;
            data->framebufferHeight = height;
            FramebufferResizeEvent event(width, height);
            data->eventCallback(event);
        });

        glfwSetWindowSizeCallback(m_Handle.get(), [](GLFWwindow* window, const int width, const int height) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");

            data->width= width;
            data->height = height;
            WindowResizeEvent event(width, height);
            data->eventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Handle.get(), [](GLFWwindow* window, const int button, const int action, const int mods) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");
            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data->eventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data->eventCallback(event);
                }
                default:
            }
        });

        glfwSetScrollCallback(m_Handle.get(), [](GLFWwindow* window, const double xoffset, const double yoffset) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");
            MouseScrolledEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
            data->eventCallback(event);
        });

        glfwSetCursorPosCallback(m_Handle.get(), [](GLFWwindow* window, const double xpos, const double ypos) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");
            MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            data->eventCallback(event);
        });

        glfwSetKeyCallback(m_Handle.get(), [](GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
            const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            VERIFY(data, "Window data is null");
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key);
                    data->eventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data->eventCallback(event);
                    break;
                }
                default:
            }
        });


        glfwGetFramebufferSize(m_Handle.get(), &m_Data.framebufferWidth, &m_Data.framebufferHeight);
        Renderer::SetViewport(m_Data.framebufferWidth, m_Data.framebufferHeight);
    }

    void Window::OnUpdate() {
        glfwSwapBuffers(m_Handle.get());
        glfwPollEvents();
    }
}
