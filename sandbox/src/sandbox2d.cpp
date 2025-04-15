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
    square_va_ = moon::ref<moon::vertex_array>(moon::vertex_array::create());
    constexpr float square_verts[3 * 4] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    uint32_t square_indices[6] = {0, 1, 2, 2, 3, 0};
    moon::ref<moon::vertex_buffer> square_vb = moon::vertex_buffer::create(&square_verts[0], sizeof(square_verts));
    square_vb->set_layout({
        { moon::ShaderDataType::Float3, "a_Pos" }
    });
    square_va_->add_vertex_buffer(square_vb);

    moon::ref<moon::index_buffer> square_ib = moon::index_buffer::create(&square_indices[0], sizeof(square_indices) / sizeof(uint32_t));
    square_va_->set_index_buffer(square_ib);

    flat_color_shader_ = moon::shader::create("assets/shaders/flat_color.glsl");

    std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->bind();
    std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->upload_uniform_float4("u_Color", square_color_);
}

void sandbox2d_layer::on_detach()
{

}

void sandbox2d_layer::on_update(moon::timestep ts)
{
    camera_controller_.on_update(ts);

    moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
    moon::render_command::clear();

    moon::renderer::begin_scene(camera_controller_.get_camera());

    std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->bind();
    std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->upload_uniform_float4("u_Color", square_color_);

    moon::renderer::submit(flat_color_shader_, square_va_, glm::mat4(1.0f));

    moon::renderer::end_scene();
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
