#include "layer_stack.h"

namespace moon
{

    layer_stack::layer_stack()
    {
        insert_iterator_ = layers_.begin();
    }

    layer_stack::~layer_stack()
    {
        for (const layer* layer : layers_)
        {
            delete layer;
        }
    }

    void layer_stack::push_layer(layer* layer)
    {
        insert_iterator_ = layers_.emplace(insert_iterator_, layer);
    }

    void layer_stack::push_overlay(layer* layer)
    {
        layers_.emplace_back(layer);
    }

    void layer_stack::pop_layer(layer* layer)
    {
        auto it = std::ranges::find(layers_, layer);
        if (it != layers_.end())
        {
            layers_.erase(it);
            --insert_iterator_;
        }
    }

    void layer_stack::pop_overlay(layer* layer)
    {
        auto it = std::ranges::find(layers_, layer);
        if (it != layers_.end())
        {
            layers_.erase(it);
        }
    }
}
