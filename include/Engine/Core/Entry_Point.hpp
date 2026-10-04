#pragma once
#include <Engine/Core/Application.hpp>
#include <Engine/Core/Log.hpp>
#include <Engine/Core/Net.hpp>


int main() {
    using namespace Engine;
    try {
        Log::Init();
        Net::Init();
        const auto app = CreateApplication();
        app->Run();
        Net::Shutdown();
    }catch (...) {
        return -1;
    }
    return 0;
}