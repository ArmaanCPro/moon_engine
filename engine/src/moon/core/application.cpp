#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "moon/events/application_event.h"
#include "moon/events/event.h"

#include "moon/imgui/imgui_layer.h"

#include "moon/renderer/renderer.h"
#include "moon/renderer/render_command.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

namespace moon
{
    application* application::s_instance = nullptr;

    application::application()
    {
        MOON_CORE_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;

        window_ = std::unique_ptr<window>(window::create());
        window_->set_event_callback([&](event& e) { on_event(e); });

        renderer::init();

        imgui_layer_ = new imgui_layer();
        push_layer(imgui_layer_);
    }

    application::~application()
    {

    }

    void application::push_layer(layer* layer)
    {
        layer_stack_.push_layer(layer);
    }

    void application::push_overlay(layer* layer)
    {
        layer_stack_.push_overlay(layer);
    }

    void application::run()
    {
        while (running_)
        {
            const auto time = (float)glfwGetTime(); // Should be Platform::GetTime
            timestep ts = time - last_frame_time_;
            last_frame_time_ = time;

            if (!minimized_)
            {
                for (layer* l : layer_stack_)
                    l->on_update(ts);
            }

            imgui_layer_->begin();
            for (layer* l : layer_stack_)
            {
                l->on_imgui_render();
            }
            imgui_layer_->end();

            window_->on_update();
        }
    }

    void application::on_event(event& e)
    {
        event_dispatcher dispatcher(e);
        dispatcher.dispatch<window_close_event>([&](window_close_event& wce) { return on_window_close(wce); });
        dispatcher.dispatch<window_resize_event>([&](window_resize_event& wre) { return on_window_resize(wre); });

        for (auto it = layer_stack_.end(); it != layer_stack_.begin(); )
        {
            (*--it)->on_event(e);
            if (e.handled)
                break;
        }
    }

    bool application::on_window_close(window_close_event&)
    {
        running_ = false;
        return true;
    }

    bool application::on_window_resize(window_resize_event& e)
    {
        // if window is minimized, it should stop running
        if (e.get_width() == 0 || e.get_height() == 0)
        {
            minimized_ = true;
            running_ = false;
            return false;
        }

        minimized_ = false;
        renderer::on_window_resize(e.get_width(), e.get_height());

        return false;
    }
}
