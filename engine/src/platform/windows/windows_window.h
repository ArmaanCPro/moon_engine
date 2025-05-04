#pragma once

#include "moon/core/window.h"

#include "platform/directx12/directx.h"

#include <glm/glm.hpp>

namespace moon
{
    class windows_window : public window
    {
    public:
        using wnd_proc_callback_fn = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

        explicit windows_window(const window_props& props);
        ~windows_window() override;

        void on_update() override;

        [[nodiscard]] uint32_t get_width() const override { return data_.width; }
        [[nodiscard]] uint32_t get_height() const override { return data_.height; }

        // window attributes
        void set_event_callback(const event_callback_fn& fn) override;
        void set_vsync(bool enabled) override;
        [[nodiscard]] bool is_vsync() const override { return data_.vsync; }

        inline void* get_native_handle() const override { return m_window_; }

        void set_fullscreen(bool enabled) override;
        [[nodiscard]] bool is_fullscreen() const override { return data_.fullscreen; }

        // set a custom callback for proc events that take priority over normal events
        void set_wnd_proc_callback(const wnd_proc_callback_fn& callback) { data_.wnd_proc_callback = callback; }

    private:
        void init(const window_props& props);
        void shutdown();

        // winapi specific
        void resize();

        static LRESULT CALLBACK on_window_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    private:
        ATOM m_window_class_ = 0;
        HWND m_window_ = nullptr;

        struct window_data
        {
            std::string title;
            uint32_t width;
            uint32_t height;
            bool vsync;
            bool fullscreen;
            bool should_resize = false;

            event_callback_fn event_callback;
            wnd_proc_callback_fn wnd_proc_callback;
        };
        window_data data_;
    };
}

