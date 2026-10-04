#pragma once
#include <memory>

#include <Engine/Core/Window.hpp>
#include <Engine/Events/Event.hpp>
#include <Engine/Events/ApplicationEvent.hpp>
#include <Engine/Core/LayerStack.hpp>


namespace Engine {

    class Application {
    public:
        Application();
        virtual ~Application() = default;
        void Run();
        void OnEvent(Event& event);
        [[nodiscard]]static const Application& Get(){
            return *s_Instance;
        }
        [[nodiscard]] const Window& GetWindow() const {
            return *m_Window;
        }
    protected:
        void PushLayer(std::unique_ptr<Layer> layer) {
            m_LayerStack.PushLayer(std::move(layer));
        }
    private:
        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);
        struct GLFW {
            GLFW();
            ~GLFW();
        } m_GLFW;
        inline static Application* s_Instance = nullptr;
        std::unique_ptr<Window> m_Window;
        LayerStack m_LayerStack;
        bool m_Running = true;
        bool m_Minimized = false;
    };

    std::unique_ptr<Application> CreateApplication();
}
