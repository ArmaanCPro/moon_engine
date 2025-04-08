#pragma once

#include "core/layer.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace moon
{
    class MOON_API imgui_layer : public layer
    {
    public:
        imgui_layer();
        ~imgui_layer() override;

        void on_attach() override;
        void on_detach() override;
        void on_update() override;
        void on_event(event& e) override;

    private:
        float time_ = 0.0f;
    };
}
