#include <GLFW/glfw3.h>
#include <array>

#include <Engine/Application.hpp>
#include <Engine/Window.hpp>
#include <Engine/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/Renderer.hpp>


namespace Engine {

    Application::Application(){
        INFO("Engine started");
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
        float lastFrame = 0.0f;
        while (m_Running){
            const auto time = static_cast<float>(glfwGetTime());
            const auto dt = time - lastFrame;
            lastFrame = time;
            m_Scene->OnUpdate(dt);
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
        m_Scene->OnEvent(event);
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