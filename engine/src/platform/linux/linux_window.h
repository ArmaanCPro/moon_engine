#pragma once

#include "core/window.h"

struct GLFWwindow;

namespace moon
{
    class linux_window : public window
    {
    public:
        explicit linux_window(const window_props& props);
        ~linux_window() override;

        void on_update() override;

        [[nodiscard]] uint32_t get_width() const override { return data_.width; }
        [[nodiscard]] uint32_t get_height() const override { return data_.height; }

        // window attributes
        void set_event_callback(const event_callback_fn& fn) override;
        void set_vsync(bool enabled) override;
        [[nodiscard]] bool is_vsync() const override { return data_.vsync; }

        inline void* get_native_window() const override { return window_; }
        inline const native_handle& get_native_handle() const override { return m_handle; }

    private:
        static void set_glfw_callbacks(GLFWwindow* window);
        GLFWwindow* window_ = nullptr;
        native_handle m_handle{};

        struct window_data
        {
            std::string title;
            uint32_t width;
            uint32_t height;
            bool vsync;

            event_callback_fn event_callback;
        };
        window_data data_;
    };
}

