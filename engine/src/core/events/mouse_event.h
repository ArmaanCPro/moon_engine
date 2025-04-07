#pragma once

#include "event.h"

namespace moon
{
    class MOON_API mouse_moved_event : public event
    {
    public:
        mouse_moved_event(float x, float y)
            :
            x_(x),
            y_(y)
        {}

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
        EVENT_CLASS_TYPE(MOUSE_MOVED_EVENT)

        inline float get_x() const { return x_; }
        inline float get_y() const { return y_; }

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MousePressedEvent: [" << x_ << ", " << y_ << "]";
            return ss.str();
        }

    private:
        float x_, y_;
    };

    class MOON_API mouse_scrolled_event : public event
    {
    public:
        mouse_scrolled_event(float x_offset, float y_offset)
            :
            x_offset_(x_offset),
            y_offset_(y_offset)
        {}

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
        EVENT_CLASS_TYPE(MOUSE_SCROLLED)

        inline float get_x_offset() const { return x_offset_; }
        inline float get_y_offset() const { return y_offset_; }

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: [" << x_offset_ << ", " << y_offset_ << "]";
            return ss.str();
        }

    private:
        float x_offset_, y_offset_;
    };

    class MOON_API mouse_button_event : public event
    {
    public:
        inline int get_mouse_button() const { return button_; }

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
    protected:
        explicit mouse_button_event(int button)
            :
            button_(button)
        {}

        int button_;
    };

    class MOON_API mouse_pressed_event : public mouse_button_event
    {
    public:
        explicit mouse_pressed_event(int button)
            :
            mouse_button_event(button)
        {}

        EVENT_CLASS_TYPE(MOUSE_BUTTON_PRESSED)

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: [" << button_;
            return ss.str();
        }
    };

    class MOON_API mouse_released_event : public mouse_button_event
    {
    public:
        explicit mouse_released_event(int button)
            :
            mouse_button_event(button)
        {}

        EVENT_CLASS_TYPE(MOUSE_BUTTON_RELEASED)

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: [" << button_;
            return ss.str();
        }
    };

}
