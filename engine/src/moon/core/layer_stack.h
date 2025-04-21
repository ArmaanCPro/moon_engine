#pragma once

#include "layer.h"

namespace moon
{
    class MOON_API layer_stack
    {
    public:
        layer_stack() = default;
        ~layer_stack();

        void push_layer(layer* layer);
        void push_overlay(layer* layer);
        void pop_layer(layer* layer);
        void pop_overlay(layer* layer);

        std::vector<layer*>::iterator begin() { return layers_.begin(); }
        std::vector<layer*>::iterator end() { return layers_.end(); }
        std::vector<layer*>::reverse_iterator rbegin() { return layers_.rbegin(); }
        std::vector<layer*>::reverse_iterator rend() { return layers_.rend(); }

        std::vector<layer*>::const_iterator begin() const { return layers_.begin(); }
        std::vector<layer*>::const_iterator end() const { return layers_.end(); }
        std::vector<layer*>::const_reverse_iterator rbegin() const { return layers_.rbegin(); }
        std::vector<layer*>::const_reverse_iterator rend() const { return layers_.rend(); }

    private:
        std::vector<layer*> layers_;
        uint32_t insert_index_ = 0;
    };
}
