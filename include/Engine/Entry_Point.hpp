#pragma once
#include <Engine/Application.hpp>
#include <Engine/Log.hpp>
#include <Engine/Net.hpp>


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