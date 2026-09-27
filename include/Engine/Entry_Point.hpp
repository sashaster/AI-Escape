#pragma once
#include <Engine/Application.hpp>
#include <Engine/Log.hpp>


int main() {
    using namespace Engine;
    try {
        Log::Init();
        const auto app = CreateApplication();
        app->Run();
    }catch (...) {
        return -1;
    }
    return 0;
}