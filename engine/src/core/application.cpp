#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "events/application_event.h"
#include "events/event.h"

namespace moon
{
    application::application()
    {

    }

    application::~application()
    {

    }

    void application::run()
    {
        window_resize_event e(1280, 720);
        MOON_TRACE("{0}", e.to_string());

        while (true)
        {}
    }
}
