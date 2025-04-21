#pragma once

#include "core.h"

namespace moon
{
    class MOON_API input
    {
    public:
        static bool is_key_pressed(KeyCode key);

        static bool is_mouse_button_pressed(MouseCode button);
        static std::pair<float, float> get_mouse_position();
        static float get_mouse_x();
        static float get_mouse_y();
    };
}
