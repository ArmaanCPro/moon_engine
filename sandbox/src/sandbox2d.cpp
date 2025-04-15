#include "sandbox2d.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

template<typename Fn>
class timer
{
public:
    timer(const char* name, Fn&& callback)
        :
        name_(name), callback_(callback), stopped_(false)
    {
        start_timepoint_ = std::chrono::high_resolution_clock::now();
    }

    ~timer()
    {
        if (!stopped_)
            stop();
    }

    void stop()
    {
        const auto end_timepoint = std::chrono::high_resolution_clock::now();

        const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint_).time_since_epoch().count();
        const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

        stopped_ = true;

        float duration = (end - start) * 0.001f;

        callback_({name_, duration});
        //MOON_TRACE("{0}: Duration {1}ms", name_, duration);
    }

private:
    const char* name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint_;
    Fn callback_;
    bool stopped_;
};

#define PROFILE_SCOPE(name) timer CONCATENATE(t_, __LINE__) (name, [&](profile_result result) { profile_results_.push_back(result); })
#define CONCATENATE(a, b) CONCATENATE_INNER(a, b)
#define CONCATENATE_INNER(a, b) a##b

sandbox2d_layer::sandbox2d_layer()
    :
    layer("Sandbox2D"),
    camera_controller_(16.0f / 9.0f, true)
{}

void sandbox2d_layer::on_attach()
{
    checkerboard_texture_ = moon::texture2d::create("assets/textures/Checkerboard.png");
}

void sandbox2d_layer::on_detach()
{

}

void sandbox2d_layer::on_update(moon::timestep ts)
{
    PROFILE_SCOPE("sandbox2d_layer::on_update");

    {
        PROFILE_SCOPE("Camera Controller On Update");
        camera_controller_.on_update(ts);
    }

    {
        PROFILE_SCOPE("Renderer Prep");
        moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
        moon::render_command::clear();
    }

    {
        PROFILE_SCOPE("Renderer Draw");
        moon::renderer2d::begin_scene(camera_controller_.get_camera());

        moon::renderer2d::draw_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.2f, 0.1f, 0.8f, 1.0f });
        moon::renderer2d::draw_quad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, square_color_);
        moon::renderer2d::draw_quad({ 0.0f, 0.0f, -0.1f }, glm::vec3(10.0f), checkerboard_texture_);

        moon::renderer2d::end_scene();
    }
}

void sandbox2d_layer::on_imgui_render()
{
    if (!ImGui::GetCurrentContext())
        ImGui::SetCurrentContext(moon_get_imgui_context());

    ImGui::Begin("Settings");

    ImGui::ColorEdit4("Square Color", glm::value_ptr(square_color_));

    for (auto& result : profile_results_)
    {
        char label[50];
        strcpy(label, "%.3fms ");
        strcat(label, result.name);
        ImGui::Text(label, result.time);
    }
    profile_results_.clear();

    ImGui::End();
}

void sandbox2d_layer::on_event(moon::event& e)
{
    camera_controller_.on_event(e);
}
