#pragma once

#include <utility>

#include "moonpch.h"
#include "moon/events/event.h"
#include "moon/renderer/graphics_context.h"

namespace moon
{
    struct window_props
    {
        explicit window_props(std::string_view title = "Moon Engine", uint32_t width = 1280, uint32_t height = 720)
            : title(title), width(width), height(height)
        {}

        std::string title;
        uint32_t width;
        uint32_t height;
    };

    // abstract class of a desktop window
    class MOON_API window
    {
    public:
        using event_callback_fn = std::function<void(event&)>;

        virtual ~window()
        {
            MOON_CORE_ASSERT(context_, "Window context is null!");
            delete context_;
            context_ = nullptr;
        }

        virtual void on_update() = 0;

        [[nodiscard]] virtual uint32_t get_width() const = 0;
        [[nodiscard]] virtual uint32_t get_height() const = 0;

        // window attributes
        virtual void set_event_callback(const event_callback_fn& fn) = 0;
        virtual void set_vsync(bool enabled) = 0;
        [[nodiscard]] virtual bool is_vsync() const = 0;

        [[nodiscard]] virtual void* get_native_window() const = 0;

        static window* create(const window_props& props = window_props());

    protected:
        graphics_context* context_ = nullptr;
    };
}
