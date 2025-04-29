#pragma once

#include "moon/core/layer.h"

namespace moon
{
    class imgui_layer : public layer
    {
    public:
        virtual ~imgui_layer() override = default;

        virtual void on_attach() override {};
        virtual void on_detach() override {};
        virtual void on_event(event&) override {};

        virtual void begin() {};
        virtual void end() {};

        void set_block_events(bool block) { m_block_events_ = block; }
    protected:
        float time_ = 0.0f;
        bool m_block_events_ = false;
    };
}
