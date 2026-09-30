#pragma once
#include <memory>
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <string_view>

#include <Engine/Log.hpp>
#include <Engine/Events/Event.hpp>


namespace Engine {

    class Window {
        using WindowHandle = std::unique_ptr<GLFWwindow, decltype([](GLFWwindow* window) {
            glfwDestroyWindow(window);
            window = nullptr;
            INFO("Window closed");
        })>;
        using EventFn = std::function<void(Event&)>;
    public:
        Window(std::string_view title, int width, int height);

        void SetEventCallback(const EventFn& callback) noexcept {
            m_Data.eventCallback = callback;
        }
        void OnUpdate();
        [[nodiscard]] GLFWwindow* GetHandle() const {
            return m_Handle.get();
        }

    private:
        void Init();

        struct WindowData {
            std::string title;
            int width;
            int height;

            EventFn eventCallback;

        } m_Data;

        WindowHandle m_Handle;
    };
}

