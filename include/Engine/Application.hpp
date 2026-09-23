#pragma once
#include <memory>

#include <Engine/Window.hpp>
#include <Engine/Events/Event.hpp>
#include <Engine/Events/ApplicationEvent.hpp>
#include <Engine/Scene.hpp>


namespace Engine {

    class Application {
    public:
        Application();
        virtual ~Application() = default;
        void Run();
        void OnEvent(Event& event);
    protected:
        void SetScene(std::unique_ptr<Scene> scene) {
            m_Scene = std::move(scene);
        }
    private:
        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);
        struct GLFW {
            GLFW();
            ~GLFW();
        } m_GLFW;
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Scene> m_Scene;
        bool m_Running = true;
        bool m_Minimized = false;
    };

    std::unique_ptr<Application> CreateApplication();
}
