#include <GLFW/glfw3.h>
#include <array>

#include <Engine/Core/Application.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Core/Core.hpp>
#include <Engine/Renderer/Buffer.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/ImGui/ImGuiLayer.hpp>


namespace Engine {

    Application::Application(){
        INFO("Starting engine...");
        s_Instance = this;
        m_Window = std::make_unique<Window>("AI-Escape", kWindowWidth, kWindowHeight);
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
        auto imgui = std::make_unique<ImGuiLayer>();
        m_ImGuiLayer = imgui.get();
        PushOverlay(std::move(imgui));
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
        auto lastFrame = glfwGetTime();
        while (m_Running){
            const auto time = glfwGetTime();
            const auto dt = static_cast<float>(time - lastFrame);
            lastFrame = time;
            m_Stats.Update(dt);
            if (!m_Minimized) {
                for (const auto& layer: m_LayerStack) {
                    layer->OnUpdate(dt);
                }

                m_ImGuiLayer->Begin(dt);
                for (const auto& layer: m_LayerStack) {
                    layer->OnImGuiRender();
                }
                m_ImGuiLayer->End();
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
        dispatcher.Dispatch<FramebufferResizeEvent>([this](FramebufferResizeEvent& e) {
            return OnFramebufferResize(e);
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
            return true;
        }
        m_Minimized = false;
        return false;
    }

    bool Application::OnFramebufferResize(FramebufferResizeEvent& event) {
        if (event.GetWidth() == 0 || event.GetHeight() == 0) {
            m_Minimized = true;
            return true;
        }
        Renderer::SetViewport(event.GetWidth(), event.GetHeight());
        m_Minimized = false;
        return false;
    }
}