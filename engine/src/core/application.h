#pragma once

#include "window.h"
#include "core/core.h"

namespace moon
{
    class window_close_event;

    class MOON_API application
    {
    public:
        application();
        virtual ~application();

        void run();

        virtual void on_event(event& e);

    private:
        bool on_window_close(window_close_event& e);

        std::unique_ptr<window> window_;
        bool running_ = true;
    };

    // to be defined in the client
    application* create_application();
}
