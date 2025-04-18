#include "sandbox2d.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <moon.h>

sandbox2d_layer::sandbox2d_layer()
    :
    layer("Sandbox2D"),
    camera_controller_(16.0f / 9.0f, true)
{}

void sandbox2d_layer::on_attach()
{
    MOON_PROFILE_FUNCTION();

    checkerboard_texture_ = moon::texture2d::create("assets/textures/Checkerboard.png");
}

void sandbox2d_layer::on_detach()
{
    MOON_PROFILE_FUNCTION();

}

void sandbox2d_layer::on_update(moon::timestep ts)
{
    MOON_PROFILE_FUNCTION();

    camera_controller_.on_update(ts);

    moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
    moon::render_command::clear();

    moon::renderer2d::begin_scene(camera_controller_.get_camera());

    //moon::renderer2d::draw_rotated_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(45.0f), { 0.2f, 0.1f, 0.8f, 1.0f });
    moon::renderer2d::draw_quad({ -1.0f, 0.0f, 0.1f }, { 0.8f, 0.8f }, { 0.2f, 0.1f, 0.8f, 1.0f });
    moon::renderer2d::draw_quad({ 0.5f, -0.5f, 0.2f }, { 0.5f, 0.75f }, square_color_);
    moon::renderer2d::draw_quad({ -5.0f, -5.0f, -0.1f }, glm::vec3(10.0f), checkerboard_texture_, 10.0f);
    moon::renderer2d::draw_quad({ 0.0f, 0.0, 0.0f }, glm::vec3(1.0f), checkerboard_texture_, 10.0f);

    moon::renderer2d::end_scene();
}

void sandbox2d_layer::on_imgui_render()
{
    MOON_PROFILE_FUNCTION();

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
