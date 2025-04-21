#pragma once

#include "moon.h"

#include <imgui.h>

namespace moon
{
    class editor_layer : public moon::layer
    {
    public:
        editor_layer();

        void on_attach() override;
        void on_detach() override;

        void on_update(timestep ts) override;

        void on_imgui_render() override;

        void on_event(event&) override;

    private:
        ref<vertex_array> square_va_;
        ref<shader> flat_color_shader_;
        ref<texture2d> checkerboard_texture_;
        ref<framebuffer> m_framebuffer_;

        glm::vec4 square_color_ { 0.8f, 0.1f, 0.2f, 1.0f };

        glm::vec2 m_viewport_size_ {0};

        orthographic_camera_controller camera_controller_;
    };
}
