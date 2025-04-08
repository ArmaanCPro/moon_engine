#pragma once

#include "window.h"
#include "core/core.h"
#include "layer.h"
#include "layer_stack.h"

namespace moon
{
    class window_close_event;

    class MOON_API application
    {
    public:
        application();
        virtual ~application();

        void push_layer(layer* layer);
        void push_overlay(layer* layer);

        void run();

        void on_event(event& e);

    private:
        bool on_window_close(window_close_event& e);

        std::unique_ptr<window> window_;
        bool running_ = true;

        layer_stack layer_stack_;
    };

    // to be defined in the client
    application* create_application();
}
