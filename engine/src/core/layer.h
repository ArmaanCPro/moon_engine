#pragma once

#include "events/event.h"

namespace moon
{
    class MOON_API layer
    {
    public:
        explicit layer(const std::string& debug_name = "layer");
        virtual ~layer() = default;

        virtual void on_attach() = 0;
        virtual void on_detach() = 0;
        virtual void on_update() = 0;
        virtual void on_event(event& e) = 0;

        inline std::string_view get_debug_name() const { return debug_name_; }
    private:
        std::string debug_name_;
    };
}
