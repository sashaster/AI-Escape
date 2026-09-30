#include <Engine/LayerStack.hpp>


namespace Engine {

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
        layer->OnAttach();
        m_Layers.push_back(std::move(layer));
    }
}