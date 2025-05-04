#pragma once

#include "event.h"
#include "core/key_codes.h"

#include <sstream>

namespace moon
{
    class MOON_API key_event : public event
    {
    public:
        inline KeyCode get_keycode() const { return m_keycode; }

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
    protected:
        explicit key_event(KeyCode keycode)
            :
            m_keycode(keycode)
        {}

        KeyCode m_keycode;
    };

    class MOON_API key_typed_event : public key_event
    {
    public:
        explicit key_typed_event(KeyCode keycode)
            :
            key_event(keycode)
        {}

        EVENT_CLASS_TYPE(KEY_TYPED)

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyTypedEvent: " << m_keycode;
            return ss.str();
        }
    };

    class MOON_API key_pressed_event : public key_event
    {
    public:
        key_pressed_event(KeyCode keycode, int repeat_count)
            :
            key_event(keycode),
            repeat_count_(repeat_count)
        {}

        EVENT_CLASS_TYPE(KEY_PRESSED)

        inline int get_repeat_count() const { return repeat_count_; }

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << m_keycode << " (" << repeat_count_ << " repeats)";
            return ss.str();
        }
    private:
        int repeat_count_;
    };

    class MOON_API key_released_event : public key_event
    {
    public:
        explicit key_released_event(KeyCode keycode)
            :
            key_event(keycode)
        {}
        EVENT_CLASS_TYPE(KEY_RELEASED)

        [[nodiscard]] std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << m_keycode;
            return ss.str();
        }
    };
}
