#include "moonpch.h"

#include "windows_window.h"

#include "core/application.h"
#include "moon/core/log.h"
#include "moon/events/application_event.h"
#include "moon/events/mouse_event.h"
#include "moon/events/key_event.h"
#include "platform/directx12/directx_context.h"
#include "platform/opengl/opengl_context.h"

#include <windows.h>
#include <directx/d3dx12.h>

namespace moon
{
    static int s_window_count = 0;

    scope<window> window::create(const window_props& props)
    {
        return create_scope<windows_window>(props);
    }

    windows_window::windows_window(const window_props& props)
    {
        MOON_PROFILE_FUNCTION();

        init(props);
    }

    windows_window::~windows_window()
    {
        MOON_PROFILE_FUNCTION();

        shutdown();
    }

    void windows_window::on_update()
    {
        MOON_PROFILE_FUNCTION();

        application::get().get_context()->swap_buffers();

        if (data_.should_resize)
            resize();

        // polling events
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void windows_window::set_event_callback(const event_callback_fn& fn)
    {
        data_.event_callback = fn;
    }

    void windows_window::set_vsync(bool enabled)
    {
        data_.vsync = enabled;
        ((directx_context*)application::get().get_context())->set_vsync(enabled);
    }

    void windows_window::set_fullscreen(bool enabled)
    {
        MOON_PROFILE_FUNCTION();

        // update window styling
        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

        if (enabled)
        {
            style = WS_POPUP | WS_VISIBLE;
            exStyle = WS_EX_TOPMOST | WS_EX_APPWINDOW;
        }

        SetWindowLong(m_window_, GWL_STYLE, style);
        SetWindowLong(m_window_, GWL_EXSTYLE, exStyle);

        // adjust window size
        HMONITOR monitor = MonitorFromWindow(m_window_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfo(monitor, &monitorInfo))
        {
            SetWindowPos(m_window_, nullptr,
                monitorInfo.rcWork.left, monitorInfo.rcWork.top,
                monitorInfo.rcWork.right - monitorInfo.rcWork.left,
                monitorInfo.rcWork.bottom - monitorInfo.rcWork.top,
                SWP_NOZORDER
            );
        }
        else
        {
            ShowWindow(m_window_, SW_MAXIMIZE);
        }

        data_.fullscreen = enabled;
    }

    void windows_window::init(const window_props& props)
    {
        MOON_PROFILE_FUNCTION();

        data_.height = props.height;
        data_.width = props.width;
        data_.title = props.title;

        MOON_CORE_INFO("Creating window {0} ({1}, {2})", data_.title, data_.width, data_.height);

        if (s_window_count == 0)
        {
            WNDCLASSEX wcex = {};
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_OWNDC;
            wcex.lpfnWndProc = on_window_message;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = GetModuleHandle(nullptr);
            wcex.hIcon = nullptr;
            wcex.hCursor = nullptr;
            wcex.hbrBackground = nullptr;
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = props.title.c_str();
            wcex.hIconSm = nullptr;
            m_window_class_ = RegisterClassEx(&wcex);
            if (!m_window_class_)
            {
                DWORD error = GetLastError();
                MOON_CORE_ERROR("Failed to register window class! Error code: {0}", error);
            }

            m_window_ = CreateWindowEx(
                WS_EX_APPWINDOW, props.title.c_str(),
                props.title.c_str(),
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, data_.width, data_.height,
                nullptr, nullptr, wcex.hInstance, &data_
            );

            if (!m_window_)
            {
                MOON_CORE_ERROR("Failed to create window!");
            }

            ShowWindow(m_window_, SW_SHOW);

            LONG_PTR result = SetWindowLongPtr(m_window_, GWLP_USERDATA, (LONG_PTR)&data_);
            if (result == 0 && GetLastError() != 0) {
                DWORD error = GetLastError();
                MOON_CORE_ERROR("Failed to set window user data! Error code: {0}", error);
            }

            ++s_window_count;
        }
    }

    void windows_window::shutdown()
    {
        MOON_PROFILE_FUNCTION();

        if (m_window_class_)
        {
            UnregisterClass((LPCSTR)m_window_class_, GetModuleHandle(nullptr));
        }

        if (m_window_)
        {
            DestroyWindow(m_window_);
            m_window_ = nullptr;
            --s_window_count;
        }
    }

    void windows_window::resize()
    {
        RECT cr = {};
        if (GetClientRect(m_window_, &cr))
        {
            data_.width = cr.right - cr.left;
            data_.height = cr.bottom - cr.top;

            ((directx_context*)application::get().get_context())->on_resize(data_.width, data_.height);
        }
    }

    static KeyCode win_to_moon_key_code(uint16_t win32KeyCode)
    {
        static const std::unordered_map<uint16_t, KeyCode> win32ToMoonMap = {
            {32,  KeyCode::Space},
            {39,  KeyCode::Apostrophe},
            {44,  KeyCode::Comma},
            {45,  KeyCode::Minus},
            {46,  KeyCode::Period},
            {47,  KeyCode::Slash},
            {48,  KeyCode::D0},
            {49,  KeyCode::D1},
            {50,  KeyCode::D2},
            {51,  KeyCode::D3},
            {52,  KeyCode::D4},
            {53,  KeyCode::D5},
            {54,  KeyCode::D6},
            {55,  KeyCode::D7},
            {56,  KeyCode::D8},
            {57,  KeyCode::D9},
            {59,  KeyCode::Semicolon},
            {61,  KeyCode::Equal},
            {65,  KeyCode::A},
            {66,  KeyCode::B},
            {67,  KeyCode::C},
            {68,  KeyCode::D},
            {69,  KeyCode::E},
            {70,  KeyCode::F},
            {71,  KeyCode::G},
            {72,  KeyCode::H},
            {73,  KeyCode::I},
            {74,  KeyCode::J},
            {75,  KeyCode::K},
            {76,  KeyCode::L},
            {77,  KeyCode::M},
            {78,  KeyCode::N},
            {79,  KeyCode::O},
            {80,  KeyCode::P},
            {81,  KeyCode::Q},
            {82,  KeyCode::R},
            {83,  KeyCode::S},
            {84,  KeyCode::T},
            {85,  KeyCode::U},
            {86,  KeyCode::V},
            {87,  KeyCode::W},
            {88,  KeyCode::X},
            {89,  KeyCode::Y},
            {90,  KeyCode::Z},
            {91,  KeyCode::LeftBracket},
            {92,  KeyCode::Backslash},
            {93,  KeyCode::RightBracket},
            {96,  KeyCode::GraveAccent},
            {256, KeyCode::Escape},
            {257, KeyCode::Enter},
            {258, KeyCode::Tab},
            {259, KeyCode::Backspace},
            {260, KeyCode::Insert},
            {261, KeyCode::Delete},
            {262, KeyCode::Right},
            {263, KeyCode::Left},
            {264, KeyCode::Down},
            {265, KeyCode::Up},
            {266, KeyCode::PageUp},
            {267, KeyCode::PageDown},
            {268, KeyCode::Home},
            {269, KeyCode::End},
            {280, KeyCode::CapsLock},
            {281, KeyCode::ScrollLock},
            {282, KeyCode::NumLock},
            {283, KeyCode::PrintScreen},
            {284, KeyCode::Pause},
            {290, KeyCode::F1},
            {291, KeyCode::F2},
            {292, KeyCode::F3},
            {293, KeyCode::F4},
            {294, KeyCode::F5},
            {295, KeyCode::F6},
            {296, KeyCode::F7},
            {297, KeyCode::F8},
            {298, KeyCode::F9},
            {299, KeyCode::F10},
            {300, KeyCode::F11},
            {301, KeyCode::F12},
            {302, KeyCode::F13},
            {303, KeyCode::F14},
            {304, KeyCode::F15},
            {305, KeyCode::F16},
            {306, KeyCode::F17},
            {307, KeyCode::F18},
            {308, KeyCode::F19},
            {309, KeyCode::F20},
            {310, KeyCode::F21},
            {311, KeyCode::F22},
            {312, KeyCode::F23},
            {313, KeyCode::F24},
            {314, KeyCode::F25},
            {320, KeyCode::KP0},
            {321, KeyCode::KP1},
            {322, KeyCode::KP2},
            {323, KeyCode::KP3},
            {324, KeyCode::KP4},
            {325, KeyCode::KP5},
            {326, KeyCode::KP6},
            {327, KeyCode::KP7},
            {328, KeyCode::KP8},
            {329, KeyCode::KP9},
            {330, KeyCode::KPDecimal},
            {331, KeyCode::KPDivide},
            {332, KeyCode::KPMultiply},
            {333, KeyCode::KPSubtract},
            {334, KeyCode::KPAdd},
            {335, KeyCode::KPEnter},
            {336, KeyCode::KPEqual},
            {340, KeyCode::LeftShift},
            {341, KeyCode::LeftControl},
            {342, KeyCode::LeftAlt},
            {343, KeyCode::LeftSuper},
            {344, KeyCode::RightShift},
            {345, KeyCode::RightControl},
            {346, KeyCode::RightAlt},
            {347, KeyCode::RightSuper},
            {348, KeyCode::Menu}
        };

        auto it = win32ToMoonMap.find(win32KeyCode);
        if (it != win32ToMoonMap.end())
        {
            return it->second;
        }

        MOON_CORE_ASSERT(false, "Unknown KeyCode!");
        return KeyCode::A;
    }

    static MouseCode win_to_moon_mouse_code(uint16_t win32MouseCode)
    {
        // Map Win32 mouse button codes to moon MouseCode
        static const std::unordered_map<uint16_t, MouseCode> win32ToMoonMouseMap = {
            {0, MouseCode::Button0},
            {1, MouseCode::Button1},
            {2, MouseCode::Button2},
            {3, MouseCode::Button3},
            {4, MouseCode::Button4},
            {5, MouseCode::Button5},
            {6, MouseCode::Button6},
            {7, MouseCode::Button7}
        };

        auto it = win32ToMoonMouseMap.find(win32MouseCode);
        if (it != win32ToMoonMouseMap.end())
        {
            return it->second;
        }
        MOON_CORE_ASSERT(false, "Unknown MouseCode!");
        return MouseCode::Button0;
    }

    LRESULT windows_window::on_window_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        window_data* data = (window_data*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        // not sure if this is needed, but why not
        if (msg == WM_CREATE)
        {
            CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)create_struct->lpCreateParams);
            return 0;
        }

        if (!data || !data->event_callback)
        {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        if (data && data->event_callback && data->wnd_proc_callback)
        {
            LRESULT handled = data->wnd_proc_callback(hWnd, msg, wParam, lParam);
            if (handled != 0)
            {
                return handled;
            }
        }

        switch (msg)
        {
        case WM_DESTROY:
        {
            window_close_event wce;
            data->event_callback(wce);
            PostQuitMessage(0);
            return 0;
        }
        case WM_CLOSE:
        {
            window_close_event wce;
            data->event_callback(wce);
            PostQuitMessage(0);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            mouse_moved_event me((float)lParam, (float)wParam);
            data->event_callback(me);
            return 0;
        }
        case WM_CHAR:
        {
            key_typed_event kte(win_to_moon_key_code((uint16_t)wParam));
            data->event_callback(kte);
            return 0;
        }
        case WM_KEYDOWN:
        {
            key_pressed_event kpe(win_to_moon_key_code((uint16_t)wParam), (int)lParam & 0xFFFF);
            data->event_callback(kpe);
            return 0;
        }
        case WM_KEYUP:
        {
            key_released_event kre(win_to_moon_key_code((uint16_t)wParam));
            data->event_callback(kre);
            return 0;
        }
        case WM_SIZE:
        {
            // don't resize on minimize/reopen
            if (lParam && (HIWORD(lParam) != data->height || HIWORD(lParam) != data->width))
            {
                return 0;
            }
            data->width = LOWORD(lParam);
            data->height = HIWORD(lParam);
            data->should_resize = true;
            window_resize_event wre(data->width, data->height);
            data->event_callback(wre);
            return 0;
        }
        case WM_HSCROLL:
        {
            mouse_scrolled_event mhse((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0);
            data->event_callback(mhse);
            return 0;
        }
        case WM_VSCROLL:
        {
            mouse_scrolled_event msve(0, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
            data->event_callback(msve);
            return 0;
        }
        case WM_MBUTTONDOWN:
        {
            mouse_pressed_event mmpe(win_to_moon_mouse_code((uint16_t)wParam));
            data->event_callback(mmpe);
            return 0;
        }
        case WM_MBUTTONUP:
        {
            mouse_released_event mmre(win_to_moon_mouse_code((uint16_t)wParam));
            data->event_callback(mmre);
            return 0;
        }
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
