#pragma once

namespace moon
{
    typedef enum class MouseCode : uint16_t
    {
        // From glfw3.h
        Button0                = 0,
        Button1                = 1,
        Button2                = 2,
        Button3                = 3,
        Button4                = 4,
        Button5                = 5,
        Button6                = 6,
        Button7                = 7,

        ButtonLast             = Button7,
        ButtonLeft             = Button0,
        ButtonRight            = Button1,
        ButtonMiddle           = Button2
    } Mouse;

    inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
    {
        os << static_cast<int32_t>(mouseCode);
        return os;
    }
}

// from glfw3.h
#define MOON_MOUSE_BUTTON_0         ::moon::MouseCode::Button0
#define MOON_MOUSE_BUTTON_1         ::moon::MouseCode::Button1
#define MOON_MOUSE_BUTTON_2         ::moon::MouseCode::Button2
#define MOON_MOUSE_BUTTON_3         ::moon::MouseCode::Button3
#define MOON_MOUSE_BUTTON_4         ::moon::MouseCode::Button4
#define MOON_MOUSE_BUTTON_5         ::moon::MouseCode::Button5
#define MOON_MOUSE_BUTTON_6         ::moon::MouseCode::Button6
#define MOON_MOUSE_BUTTON_7         ::moon::MouseCode::Button7
#define MOON_MOUSE_BUTTON_LAST      ::moon::MouseCode::ButtonLast
#define MOON_MOUSE_BUTTON_LEFT      ::moon::MouseCode::ButtonLeft
#define MOON_MOUSE_BUTTON_RIGHT     ::moon::MouseCode::ButtonRight
#define MOON_MOUSE_BUTTON_MIDDLE    ::moon::MouseCode::ButtonMiddle
