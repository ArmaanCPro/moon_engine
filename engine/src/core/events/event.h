#pragma once

#include "core/core.h"


namespace moon
{
    enum class event_type
    {
        NONE,
        WINDOW_CLOSE,
        WINDOW_RESIZE,
        WINDOW_FOCUS,
        WINDOW_LOST_FOCUS,
        APP_TICK,
        APP_UPDATE,
        APP_RENDER,
        KEY_TYPED,
        KEY_PRESSED,
        KEY_RELEASED,
        MOUSE_BUTTON_PRESSED,
        MOUSE_BUTTON_RELEASED,
        MOUSE_SCROLLED,
        MOUSE_MOVED_EVENT
    };

    enum event_category
    {
        NONE                        = 0,
        EVENT_CATEGORY_APPLICATION  = BIT(0),
        EVENT_CATEGORY_INPUT        = BIT(1),
        EVENT_CATEGORY_KEYBOARD     = BIT(2),
        EVENT_CATEGORY_MOUSE        = BIT(3),
        EVENT_CATEGORY_MOUSE_BUTTON = BIT(4)
    };

#define EVENT_CLASS_TYPE(type) static event_type get_static_type() { return event_type::type; }\
                                virtual event_type get_type() const override { return get_static_type(); }\
                                virtual const char* get_name() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int get_category() const override { return category; }

    // TODO consider replacing the key event logic seen here with something more akin to unreal's
    // i.e., an event has a state of started, triggered, stopped, ongoing, etc

    class MOON_API event
    {
        friend class event_dispatcher;
    public:
        virtual ~event() = default;

        bool handled = false;

        [[nodiscard]] virtual event_type get_type() const = 0;
        [[nodiscard]] virtual const char* get_name() const = 0;
        [[nodiscard]] virtual int get_category() const = 0;
        [[nodiscard]] virtual std::string to_string() const { return get_name(); }

        [[nodiscard]] inline bool is_in_category(event_category category) const
        {
            return get_category() & category;
        }
    private:
    };

    class event_dispatcher
    {
        template<typename T>
        using event_fn = std::function<bool(T&)>;

    public:
        explicit event_dispatcher(event& event)
            :
            event_(event)
        {}

        template<typename T>
        bool dispatch(event_fn<T> func)
        {
            if (event_.get_type() == T::get_static_type())
            {
                event_.handled = func(*(T*)&event_);
                return true;
            }
            return false;
        }
    private:
        event& event_;
    };

    inline std::ostream& operator<<(std::ostream& os, const event& e)
    {
        return os << e.to_string();
    }
}
