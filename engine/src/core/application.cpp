#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "events/application_event.h"
#include "events/event.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace moon
{
    application::application()
    {
        window_ = std::unique_ptr<window>(window::create());

        window_->set_event_callback(std::bind(&application::on_event, this, std::placeholders::_1));
    }

    application::~application()
    {
    }

    void application::push_layer(layer* layer)
    {
        layer_stack_.push_layer(layer);
        layer->on_attach();
    }

    void application::push_overlay(layer* layer)
    {
        layer_stack_.push_overlay(layer);
        layer->on_attach();
    }

    void application::run()
    {
        while (running_)
        {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (layer* l : layer_stack_)
                l->on_update();

            window_->on_update();
        }
    }

    void application::on_event(event& e)
    {
        event_dispatcher dispatcher(e);
        dispatcher.dispatch<window_close_event>(std::bind(&application::on_window_close, this, std::placeholders::_1));

        for (auto it = layer_stack_.end(); it != layer_stack_.begin(); )
        {
            (*--it)->on_event(e);
            if (e.handled)
                break;
        }

        MOON_CORE_TRACE("event: {0}", e.to_string());
    }

    bool application::on_window_close(window_close_event&)
    {
        running_ = false;
        return true;
    }
}
