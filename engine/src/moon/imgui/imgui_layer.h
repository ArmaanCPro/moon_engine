#pragma once

#include "moon/core/layer.h"

namespace moon
{
    class imgui_layer final : public layer
    {
    public:
        imgui_layer();
        ~imgui_layer() override = default;

        void on_attach() override;
        void on_detach() override;
        void on_event(event&) override;

        void begin();
        void end();

        void set_block_events(bool block) { m_block_events_ = block; }
    private:
        bool m_block_events_ = false;
        float time_ = 0.0f;
    };
}
