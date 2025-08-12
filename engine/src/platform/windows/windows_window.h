#pragma once

#include "moon/core/window.h"

struct GLFWwindow;

namespace moon
{
    class windows_window : public window
    {
    public:
        explicit windows_window(const window_props& props);
        ~windows_window() override;

        void on_update() override;

        [[nodiscard]] uint32_t get_width() const override { return data_.width; }
        [[nodiscard]] uint32_t get_height() const override { return data_.height; }

        // window attributes
        void set_event_callback(const event_callback_fn& fn) override;
        void set_vsync(bool enabled) override;
        [[nodiscard]] bool is_vsync() const override { return data_.vsync; }

        [[nodiscard]] inline native_handle get_native_handle() const override { return window_; }

    private:
        static void set_glfw_callbacks(GLFWwindow* window);
        GLFWwindow* window_ = nullptr;

        struct window_data
        {
            std::string title{};
            uint32_t width{};
            uint32_t height{};
            bool vsync = false;

            event_callback_fn event_callback;
        };
        window_data data_;
    };
}

