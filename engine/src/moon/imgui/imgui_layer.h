#pragma once

#include "moon/core/layer.h"

namespace moon
{
    class MOON_API imgui_layer : public layer
    {
    public:
        imgui_layer();
        ~imgui_layer() override = default;

        void on_attach() override;
        void on_detach() override;

        void begin();
        void end();

    private:
        float time_ = 0.0f;
    };
}
