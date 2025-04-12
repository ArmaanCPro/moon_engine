#pragma once

#include "window.h"
#include "core/core.h"
#include "layer.h"
#include "layer_stack.h"
#include "imgui/imgui_layer.h"

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

        inline static application& get() { return *s_instance; }
        inline window& get_window() { return *window_; }

    private:
        bool on_window_close(window_close_event& e);

        std::unique_ptr<window> window_;
        imgui_layer* imgui_layer_;
        bool running_ = true;

        layer_stack layer_stack_;

        static application* s_instance;
    };

    // to be defined in the client
    application* create_application();
}
