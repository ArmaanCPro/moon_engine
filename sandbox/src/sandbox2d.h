#pragma once

#include <moon.h>

class sandbox2d_layer : public moon::layer
{
public:
    sandbox2d_layer();
    ~sandbox2d_layer() override = default;

    void on_attach() override;
    void on_detach() override;
    void on_update(moon::timestep ts) override;
    void on_imgui_render() override;
    void on_event(moon::event& e) override;

private:
    // temp
    moon::ref<moon::vertex_array> square_va_;
    moon::ref<moon::shader> flat_color_shader_;
    moon::ref<moon::texture2d> checkerboard_texture_;
    moon::ref<moon::framebuffer> m_framebuffer_;

    glm::vec4 square_color_ { 0.8f, 0.1f, 0.2f, 1.0f };

    moon::orthographic_camera_controller camera_controller_;

};
