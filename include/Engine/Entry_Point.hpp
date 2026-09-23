#pragma once
#include <Engine/Application.hpp>
#include <Engine/Log.hpp>


int main() {
    using namespace Engine;
    Log::Init();
    const auto app = CreateApplication();
    app->Run();
    return 0;
}