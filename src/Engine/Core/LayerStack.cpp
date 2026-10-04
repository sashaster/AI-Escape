#include <Engine/Core/LayerStack.hpp>


namespace Engine {

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
        m_Layers.push_back(std::move(layer));
    }
}