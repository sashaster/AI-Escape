#pragma once
#include <vector>


#include <Engine/Core/Layer.hpp>


namespace Engine {

    class LayerStack {
    public:
        void PushLayer(std::unique_ptr<Layer> layer);
        void PushOverlay(std::unique_ptr<Layer> layer);

        auto end() {
            return m_Layers.end();
        }

        auto begin() {
            return m_Layers.begin();
        }

        auto rbegin() {
            return m_Layers.rbegin();
        }

        auto rend() {
            return m_Layers.rend();
        }

    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        int m_InsertIndex = 0;



    };
}