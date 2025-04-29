#pragma once

#include "moon/imgui/imgui_layer.h"

namespace moon
{
    class opengl_imgui_layer : public imgui_layer
    {
    public:
        opengl_imgui_layer();
        ~opengl_imgui_layer() override;

        void on_attach() override;
        void on_detach() override;
        void on_imgui_render() override;
        void on_event(event& e) override;

        void begin() override;
        void end() override;
    };
}
