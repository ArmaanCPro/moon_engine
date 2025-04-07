#pragma once

#include "event.h"

#include <sstream>

namespace moon
{
    class MOON_API key_event : public event
    {
    public:
        inline int get_keycode() const { return keycode_; }

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
    protected:
        explicit key_event(int keycode)
            :
            keycode_(keycode)
        {}

        int keycode_ = 0;
    };

    class MOON_API key_pressed_event : public key_event
    {
    public:
        key_pressed_event(int keycode, int repeat_count)
            :
            key_event(keycode),
            repeat_count_(repeat_count)
        {}

        EVENT_CLASS_TYPE(KEY_PRESSED)

        inline int get_repeat_count() const { return repeat_count_; }

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << keycode_ << " (" << repeat_count_ << " repeats)";
            return ss.str();
        }
    private:
        int repeat_count_;
    };

    class MOON_API key_released_event : public key_event
    {
    public:
        key_released_event(int keycode)
            :
            key_event(keycode)
        {}
        EVENT_CLASS_TYPE(KEY_RELEASED)

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << keycode_;
            return ss.str();
        }
    };
}
