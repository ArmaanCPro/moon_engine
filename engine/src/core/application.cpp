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
    }

    application::~application()
    {

    }

    void application::run()
    {
        while (running_)
        {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            window_->on_update();
        }
    }
}
