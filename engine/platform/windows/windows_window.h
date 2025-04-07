#pragma once

#include "core/window.h"

#include <glfw/glfw3.h>

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

    private:
        void init(const window_props& props);
        void shutdown();
    private:
        GLFWwindow* window_;

        struct window_data
        {
            std::string title;
            uint32_t width;
            uint32_t height;
            bool vsync;
        };
        window_data data_;
    };
}

