#pragma once

#include "event.h"

#include <sstream>

namespace moon
{
    class MOON_API window_resize_event : public event
    {
    public:
        window_resize_event(uint32_t width, uint32_t height)
            :
            width_(width),
            height_(height)
        {}

        EVENT_CLASS_TYPE(WINDOW_RESIZE)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)

        inline uint32_t get_width() const { return width_; }
        inline uint32_t get_height() const { return height_; }

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << width_ << ", " << height_;
            return ss.str();
        }

    private:
        uint32_t width_, height_;
    };

    class MOON_API window_close_event : public event
    {
    public:
        window_close_event() = default;

        EVENT_CLASS_TYPE(WINDOW_CLOSE)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class MOON_API window_focus_event : public event
    {
    public:
        window_focus_event() = default;

        EVENT_CLASS_TYPE(WINDOW_FOCUS)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class MOON_API window_lost_focus_event : public event
    {
    public:
        window_lost_focus_event() = default;

        EVENT_CLASS_TYPE(WINDOW_LOST_FOCUS)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class MOON_API app_tick_event : public event
    {
    public:
        app_tick_event() = default;

        EVENT_CLASS_TYPE(APP_TICK)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class MOON_API app_update_event : public event
    {
    public:
        app_update_event() = default;

        EVENT_CLASS_TYPE(APP_UPDATE)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class MOON_API app_render_event : public event
    {
    public:
        app_render_event() = default;

        EVENT_CLASS_TYPE(APP_RENDER)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };
}
