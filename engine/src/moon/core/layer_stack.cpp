#include "moonpch.h"

#include "moon/core/layer_stack.h"

namespace moon
{
    layer_stack::~layer_stack()
    {
        for (const layer* layer : layers_)
        {
            delete layer;
        }
    }

    void layer_stack::push_layer(layer* layer)
    {
        layers_.emplace(layers_.begin() + insert_index_, layer);
        insert_index_++;
    }

    void layer_stack::push_overlay(layer* layer)
    {
        layers_.emplace_back(layer);
    }

    void layer_stack::pop_layer(layer* layer)
    {
        auto it = std::ranges::find(layers_, layer);
        if (it != layers_.begin() + insert_index_)
        {
            layer->on_detach();
            layers_.erase(it);
            --insert_index_;
        }
    }

    void layer_stack::pop_overlay(layer* layer)
    {
        auto it = std::ranges::find(layers_, layer);
        if (it != layers_.end())
        {
            layer->on_detach();
            layers_.erase(it);
        }
    }
}
