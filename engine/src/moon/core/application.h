#pragma once

#include "moon/core/window.h"
#include "moon/core/core.h"
#include "moon/core/layer.h"
#include "moon/core/layer_stack.h"
#include "moon/imgui/imgui_layer.h"

#include "moon/renderer/vertex_array.h"

namespace moon
{
    class window_close_event;
    class window_resize_event;

    class MOON_API application
    {
    public:
        application(std::string_view name = "Moon Engine");
        virtual ~application();

        void push_layer(layer* layer);
        void push_overlay(layer* layer);

        void run();

        void on_event(event& e);

        void close();

        inline static application& get() { return *s_instance; }
        inline window& get_window() { return *window_; }

    private:
        bool on_window_close(window_close_event& e);
        bool on_window_resize(window_resize_event& e);

        std::unique_ptr<window> window_;
        imgui_layer* imgui_layer_;
        bool running_ = true;
        bool minimized_ = false;

        layer_stack layer_stack_;

        static application* s_instance;

        float last_frame_time_ = 0.0f;
    };

    // to be defined in the client
    application* create_application();
}
