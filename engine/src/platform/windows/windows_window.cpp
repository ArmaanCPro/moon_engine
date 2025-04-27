#include "moonpch.h"

#include "windows_window.h"

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

        ((directx_context*)context_)->begin_frame();
        ((directx_context*)context_)->end_frame();

        context_->swap_buffers();
        ((directx_context*)context_)->execute_command_list();

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
        ((directx_context*)context_)->set_vsync(enabled);
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

        context_ = new directx_context(m_window_);
        context_->init();

        set_vsync(true);
    }

    void windows_window::shutdown()
    {
        MOON_PROFILE_FUNCTION();

        // same count as we pass in for sync interval (frames in flight)
        context_->flush(2);

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

        // TODO: Consider making shutdown a virtual func of graphics_context
        ((directx_context*)context_)->shutdown();
    }

    void windows_window::resize()
    {
        RECT cr = {};
        if (GetClientRect(m_window_, &cr))
        {
            data_.width = cr.right - cr.left;
            data_.height = cr.bottom - cr.top;

            ((directx_context*)context_)->on_resize(data_.width, data_.height);
        }
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
            key_typed_event kte((int)wParam);
            data->event_callback(kte);
            return 0;
        }
        case WM_KEYDOWN:
        {
            key_pressed_event kpe((int)wParam, (int)lParam & 0xFFFF);
            data->event_callback(kpe);
            return 0;
        }
        case WM_KEYUP:
        {
            key_released_event kre((int)wParam);
            data->event_callback(kre);
            return 0;
        }
        case WM_SIZE:
        {
            // don't resize on minimize/reopen
            if (lParam && (HIWORD(lParam) != data->height || HIWORD(lParam) != data->width))
            {
                data->should_resize = true;
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
            mouse_pressed_event mmpe((int)wParam);
            data->event_callback(mmpe);
            return 0;
        }
        case WM_MBUTTONUP:
        {
            mouse_released_event mmre((int)wParam);
            data->event_callback(mmre);
            return 0;
        }
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
