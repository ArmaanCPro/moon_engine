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

    moon::renderer2d::reset_stats();
    {
        MOON_PROFILE_SCOPE("Renderer Prep");

        moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
        moon::render_command::clear();
    }

    {
        static float rotation = 0.0f;
        rotation += ts * 50.0f;

#if 0
        MOON_PROFILE_SCOPE("Renderer Draw");
        moon::renderer2d::begin_scene(camera_controller_.get_camera());

        moon::renderer2d::draw_rotated_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(45.0f), { 0.2f, 0.1f, 0.8f, 1.0f });
        moon::renderer2d::draw_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.2f, 0.1f, 0.8f, 1.0f });
        moon::renderer2d::draw_quad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, square_color_);
        moon::renderer2d::draw_quad({ 0.0f, 0.0f, -0.1f }, glm::vec2(20.0f), checkerboard_texture_, 10.0f);
        moon::renderer2d::draw_rotated_quad({ -2.0f, 0.0 }, glm::vec2(1.0f), glm::radians(rotation), checkerboard_texture_, 10.0f);

        for (float y = -5.0f; y < 5.0f; y += 0.5f)
        {
            for (float x = -5.0f; x < 5.0f; x += 0.5f)
            {
                glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
                moon::renderer2d::draw_quad({ x, y }, { 0.45f, 0.45f }, color);
            }
        }

        moon::renderer2d::end_scene();
#endif
    }
}

void sandbox2d_layer::on_imgui_render()
{
    MOON_PROFILE_FUNCTION();

    if (!ImGui::GetCurrentContext())
        ImGui::SetCurrentContext(moon_get_imgui_context());

    ImGui::Begin("Settings");

    auto stats = moon::renderer2d::get_stats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.draw_calls);
    ImGui::Text("Quads: %d", stats.quad_count);
    ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
    ImGui::Text("Indices: %d", stats.get_total_index_count());

    ImGui::ColorEdit4("Square Color", glm::value_ptr(square_color_));
    ImGui::End();
}

void sandbox2d_layer::on_event(moon::event& e)
{
    camera_controller_.on_event(e);
}
