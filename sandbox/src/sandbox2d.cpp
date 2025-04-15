//
// Created by Armaan Chahal on 4/14/2025.
//

#include "sandbox2d.h"

#include <platform/opengl/opengl_shader.h>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

sandbox2d_layer::sandbox2d_layer()
    :
    layer("Sandbox2D"),
    camera_controller_(16.0f / 9.0f, true)
{}

void sandbox2d_layer::on_attach()
{

}

void sandbox2d_layer::on_detach()
{

}

void sandbox2d_layer::on_update(moon::timestep ts)
{
    camera_controller_.on_update(ts);

    moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
    moon::render_command::clear();

    moon::renderer2d::begin_scene(camera_controller_.get_camera());

    moon::renderer2d::draw_quad(glm::vec3(0.0f), glm::vec2(1.0f), square_color_);

    moon::renderer2d::end_scene();
    //std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->bind();
    //std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->upload_uniform_float4("u_Color", square_color_);
}

void sandbox2d_layer::on_imgui_render()
{
    if (!ImGui::GetCurrentContext())
        ImGui::SetCurrentContext(moon_get_imgui_context());

    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(square_color_));
    ImGui::End();
}

void sandbox2d_layer::on_event(moon::event& e)
{
    camera_controller_.on_event(e);
}
