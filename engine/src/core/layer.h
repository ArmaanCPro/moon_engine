#pragma once

#include "events/event.h"

namespace moon
{
    class MOON_API layer
    {
    public:
        explicit layer(std::string debug_name = "layer");
        virtual ~layer() = default;

        virtual void on_attach() {};
        virtual void on_detach() {};
        virtual void on_update() {};
        virtual void on_imgui_render() {};
        virtual void on_event(event&) {};

        inline std::string_view get_debug_name() const { return debug_name_; }
    private:
        std::string debug_name_;
    };
}
