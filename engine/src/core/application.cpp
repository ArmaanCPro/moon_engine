#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "events/application_event.h"
#include "events/event.h"

#include "imgui/imgui_layer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace moon
{
    application* application::s_instance = nullptr;

    application::application()
    {
        MOON_CORE_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;

        window_ = std::unique_ptr<window>(window::create());
        window_->set_event_callback([&](event& e) { on_event(e); });

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
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (layer* l : layer_stack_)
                l->on_update();

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
}
