#include <Engine/Core/LayerStack.hpp>


namespace Engine {

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
        m_Layers.insert(m_Layers.begin() + m_InsertIndex, std::move(layer));
        ++m_InsertIndex;
    }

    void LayerStack::PushOverlay(std::unique_ptr<Layer> layer) {
        m_Layers.push_back(std::move(layer));
    }
}