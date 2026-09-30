#pragma once
#include <Engine/Application.hpp>
#include <GLFW/glfw3.h>


namespace Engine::Input {

    [[nodiscard]] inline bool IsKeyPressed(const int keyCode) {
        const auto state = glfwGetKey(Application::Get().GetWindow().GetHandle(), keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

}