#pragma once

namespace moon
{
    typedef enum class KeyCode : uint16_t
    {
        // From glfw3.h
        Space               = 32,
        Apostrophe          = 39, /* ' */
        Comma               = 44, /* , */
        Minus               = 45, /* - */
        Period              = 46, /* . */
        Slash               = 47, /* / */

        D0                  = 48, /* 0 */
        D1                  = 49, /* 1 */
        D2                  = 50, /* 2 */
        D3                  = 51, /* 3 */
        D4                  = 52, /* 4 */
        D5                  = 53, /* 5 */
        D6                  = 54, /* 6 */
        D7                  = 55, /* 7 */
        D8                  = 56, /* 8 */
        D9                  = 57, /* 9 */

        Semicolon           = 59, /* ; */
        Equal               = 61, /* = */

        A                   = 65,
        B                   = 66,
        C                   = 67,
        D                   = 68,
        E                   = 69,
        F                   = 70,
        G                   = 71,
        H                   = 72,
        I                   = 73,
        J                   = 74,
        K                   = 75,
        L                   = 76,
        M                   = 77,
        N                   = 78,
        O                   = 79,
        P                   = 80,
        Q                   = 81,
        R                   = 82,
        S                   = 83,
        T                   = 84,
        U                   = 85,
        V                   = 86,
        W                   = 87,
        X                   = 88,
        Y                   = 89,
        Z                   = 90,

        LeftBracket         = 91,  /* [ */
        Backslash           = 92,  /* \ */
        RightBracket        = 93,  /* ] */
        GraveAccent         = 96,  /* ` */

        World1              = 161, /* non-US #1 */
        World2              = 162, /* non-US #2 */

        /* Function keys */
        Escape              = 256,
        Enter               = 257,
        Tab                 = 258,
        Backspace           = 259,
        Insert              = 260,
        Delete              = 261,
        Right               = 262,
        Left                = 263,
        Down                = 264,
        Up                  = 265,
        PageUp              = 266,
        PageDown            = 267,
        Home                = 268,
        End                 = 269,
        CapsLock            = 280,
        ScrollLock          = 281,
        NumLock             = 282,
        PrintScreen         = 283,
        Pause               = 284,
        F1                  = 290,
        F2                  = 291,
        F3                  = 292,
        F4                  = 293,
        F5                  = 294,
        F6                  = 295,
        F7                  = 296,
        F8                  = 297,
        F9                  = 298,
        F10                 = 299,
        F11                 = 300,
        F12                 = 301,
        F13                 = 302,
        F14                 = 303,
        F15                 = 304,
        F16                 = 305,
        F17                 = 306,
        F18                 = 307,
        F19                 = 308,
        F20                 = 309,
        F21                 = 310,
        F22                 = 311,
        F23                 = 312,
        F24                 = 313,
        F25                 = 314,

        /* Keypad */
        KP0                 = 320,
        KP1                 = 321,
        KP2                 = 322,
        KP3                 = 323,
        KP4                 = 324,
        KP5                 = 325,
        KP6                 = 326,
        KP7                 = 327,
        KP8                 = 328,
        KP9                 = 329,
        KPDecimal           = 330,
        KPDivide            = 331,
        KPMultiply          = 332,
        KPSubtract          = 333,
        KPAdd               = 334,
        KPEnter             = 335,
        KPEqual             = 336,

        LeftShift           = 340,
        LeftControl         = 341,
        LeftAlt             = 342,
        LeftSuper           = 343,
        RightShift          = 344,
        RightControl        = 345,
        RightAlt            = 346,
        RightSuper          = 347,
        Menu                = 348
    } Key;

    inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
    {
        os << static_cast<int32_t>(keyCode);
        return os;
    }
}

// from glfw3.h
/* Printable keys */
#define MOON_KEY_SPACE           ::moon::Key::Space
#define MOON_KEY_APOSTROPHE      ::moon::Key::Apostrophe    /* ' */
#define MOON_KEY_COMMA           ::moon::Key::Comma         /* , */
#define MOON_KEY_MINUS           ::moon::Key::Minus         /* - */
#define MOON_KEY_PERIOD          ::moon::Key::Period        /* . */
#define MOON_KEY_SLASH           ::moon::Key::Slash         /* / */
#define MOON_KEY_0               ::moon::Key::D0
#define MOON_KEY_1               ::moon::Key::D1
#define MOON_KEY_2               ::moon::Key::D2
#define MOON_KEY_3               ::moon::Key::D3
#define MOON_KEY_4               ::moon::Key::D4
#define MOON_KEY_5               ::moon::Key::D5
#define MOON_KEY_6               ::moon::Key::D6
#define MOON_KEY_7               ::moon::Key::D7
#define MOON_KEY_8               ::moon::Key::D8
#define MOON_KEY_9               ::moon::Key::D9
#define MOON_KEY_SEMICOLON       ::moon::Key::Semicolon     /* ; */
#define MOON_KEY_EQUAL           ::moon::Key::Equal         /* = */
#define MOON_KEY_A               ::moon::Key::A
#define MOON_KEY_B               ::moon::Key::B
#define MOON_KEY_C               ::moon::Key::C
#define MOON_KEY_D               ::moon::Key::D
#define MOON_KEY_E               ::moon::Key::E
#define MOON_KEY_F               ::moon::Key::F
#define MOON_KEY_G               ::moon::Key::G
#define MOON_KEY_H               ::moon::Key::H
#define MOON_KEY_I               ::moon::Key::I
#define MOON_KEY_J               ::moon::Key::J
#define MOON_KEY_K               ::moon::Key::K
#define MOON_KEY_L               ::moon::Key::L
#define MOON_KEY_M               ::moon::Key::M
#define MOON_KEY_N               ::moon::Key::N
#define MOON_KEY_O               ::moon::Key::O
#define MOON_KEY_P               ::moon::Key::P
#define MOON_KEY_Q               ::moon::Key::Q
#define MOON_KEY_R               ::moon::Key::R
#define MOON_KEY_S               ::moon::Key::S
#define MOON_KEY_T               ::moon::Key::T
#define MOON_KEY_U               ::moon::Key::U
#define MOON_KEY_V               ::moon::Key::V
#define MOON_KEY_W               ::moon::Key::W
#define MOON_KEY_X               ::moon::Key::X
#define MOON_KEY_Y               ::moon::Key::Y
#define MOON_KEY_Z               ::moon::Key::Z
#define MOON_KEY_LEFT_BRACKET    ::moon::Key::LeftBracket   /* [ */
#define MOON_KEY_BACKSLASH       ::moon::Key::Backslash     /* \ */
#define MOON_KEY_RIGHT_BRACKET   ::moon::Key::RightBracket  /* ] */
#define MOON_KEY_GRAVE_ACCENT    ::moon::Key::GraveAccent   /* ` */
#define MOON_KEY_WORLD_1         ::moon::Key::World1        /* non-US #1 */
#define MOON_KEY_WORLD_2         ::moon::Key::World2        /* non-US #2 */

/* Function keys */
#define MOON_KEY_ESCAPE          ::moon::Key::Escape
#define MOON_KEY_ENTER           ::moon::Key::Enter
#define MOON_KEY_TAB             ::moon::Key::Tab
#define MOON_KEY_BACKSPACE       ::moon::Key::Backspace
#define MOON_KEY_INSERT          ::moon::Key::Insert
#define MOON_KEY_DELETE          ::moon::Key::Delete
#define MOON_KEY_RIGHT           ::moon::Key::Right
#define MOON_KEY_LEFT            ::moon::Key::Left
#define MOON_KEY_DOWN            ::moon::Key::Down
#define MOON_KEY_UP              ::moon::Key::Up
#define MOON_KEY_PAGE_UP         ::moon::Key::PageUp
#define MOON_KEY_PAGE_DOWN       ::moon::Key::PageDown
#define MOON_KEY_HOME            ::moon::Key::Home
#define MOON_KEY_END             ::moon::Key::End
#define MOON_KEY_CAPS_LOCK       ::moon::Key::CapsLock
#define MOON_KEY_SCROLL_LOCK     ::moon::Key::ScrollLock
#define MOON_KEY_NUM_LOCK        ::moon::Key::NumLock
#define MOON_KEY_PRINT_SCREEN    ::moon::Key::PrintScreen
#define MOON_KEY_PAUSE           ::moon::Key::Pause
#define MOON_KEY_F1              ::moon::Key::F1
#define MOON_KEY_F2              ::moon::Key::F2
#define MOON_KEY_F3              ::moon::Key::F3
#define MOON_KEY_F4              ::moon::Key::F4
#define MOON_KEY_F5              ::moon::Key::F5
#define MOON_KEY_F6              ::moon::Key::F6
#define MOON_KEY_F7              ::moon::Key::F7
#define MOON_KEY_F8              ::moon::Key::F8
#define MOON_KEY_F9              ::moon::Key::F9
#define MOON_KEY_F10             ::moon::Key::F10
#define MOON_KEY_F11             ::moon::Key::F11
#define MOON_KEY_F12             ::moon::Key::F12
#define MOON_KEY_F13             ::moon::Key::F13
#define MOON_KEY_F14             ::moon::Key::F14
#define MOON_KEY_F15             ::moon::Key::F15
#define MOON_KEY_F16             ::moon::Key::F16
#define MOON_KEY_F17             ::moon::Key::F17
#define MOON_KEY_F18             ::moon::Key::F18
#define MOON_KEY_F19             ::moon::Key::F19
#define MOON_KEY_F20             ::moon::Key::F20
#define MOON_KEY_F21             ::moon::Key::F21
#define MOON_KEY_F22             ::moon::Key::F22
#define MOON_KEY_F23             ::moon::Key::F23
#define MOON_KEY_F24             ::moon::Key::F24
#define MOON_KEY_F25             ::moon::Key::F25

/* Keypad */
#define MOON_KEY_KP_0            ::moon::Key::KP0
#define MOON_KEY_KP_1            ::moon::Key::KP1
#define MOON_KEY_KP_2            ::moon::Key::KP2
#define MOON_KEY_KP_3            ::moon::Key::KP3
#define MOON_KEY_KP_4            ::moon::Key::KP4
#define MOON_KEY_KP_5            ::moon::Key::KP5
#define MOON_KEY_KP_6            ::moon::Key::KP6
#define MOON_KEY_KP_7            ::moon::Key::KP7
#define MOON_KEY_KP_8            ::moon::Key::KP8
#define MOON_KEY_KP_9            ::moon::Key::KP9
#define MOON_KEY_KP_DECIMAL      ::moon::Key::KPDecimal
#define MOON_KEY_KP_DIVIDE       ::moon::Key::KPDivide
#define MOON_KEY_KP_MULTIPLY     ::moon::Key::KPMultiply
#define MOON_KEY_KP_SUBTRACT     ::moon::Key::KPSubtract
#define MOON_KEY_KP_ADD          ::moon::Key::KPAdd
#define MOON_KEY_KP_ENTER        ::moon::Key::KPEnter
#define MOON_KEY_KP_EQUAL        ::moon::Key::KPEqual

#define MOON_KEY_LEFT_SHIFT      ::moon::Key::LeftShift
#define MOON_KEY_LEFT_CONTROL    ::moon::Key::LeftControl
#define MOON_KEY_LEFT_ALT        ::moon::Key::LeftAlt
#define MOON_KEY_LEFT_SUPER      ::moon::Key::LeftSuper
#define MOON_KEY_RIGHT_SHIFT     ::moon::Key::RightShift
#define MOON_KEY_RIGHT_CONTROL   ::moon::Key::RightControl
#define MOON_KEY_RIGHT_ALT       ::moon::Key::RightAlt
#define MOON_KEY_RIGHT_SUPER     ::moon::Key::RightSuper
#define MOON_KEY_MENU            ::moon::Key::Menu
