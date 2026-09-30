#include <GLFW/glfw3.h>
#include <array>

#include <Engine/Application.hpp>
#include <Engine/Window.hpp>
#include <Engine/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/Renderer.hpp>


namespace Engine {

    Application::Application(){
        INFO("Starting engine...");
        s_Instance = this;
        m_Window = std::make_unique<Window>("AI-Escape", WindowWidth, WindowHeight);
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
    }

    Application::GLFW::~GLFW() {
        glfwTerminate();
    }

    Application::GLFW::GLFW() {
        glfwSetErrorCallback([](int error, const char* description) {
            ERROR("GLFW error ({0}): {1}", error, description);
        });
        ASSERT(glfwInit(), "Failed to initialize GLFW");
    }

    void Application::Run() {
        INFO("Engine is running");
        float lastFrame = 0.0f;
        while (m_Running){
            const auto time = static_cast<float>(glfwGetTime());
            const auto dt = time - lastFrame;
            lastFrame = time;
            if (!m_Minimized) {
                for (const auto& layer: m_LayerStack) {
                    layer->OnUpdate(dt);
                }
            }
            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event &event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) {
            return OnWindowClose(e);
        });
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
            return OnWindowResize(e);
        });
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
            (*it)->OnEvent(event);
            if (event.IsHandled()) {
                break;
            }
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& event) {
        m_Running = false;
        INFO("Stopping Engine...");
        return false;
    }

    bool Application::OnWindowResize(WindowResizeEvent& event) {
        if (event.GetWidth() == 0 || event.GetHeight() == 0) {
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;
        Renderer::SetViewport(event.GetWidth(), event.GetHeight());
        return false;
    }
}