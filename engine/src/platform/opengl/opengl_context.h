#pragma once

#include "moon/renderer/graphics_context.h"

struct GLFWwindow;

namespace moon
{
    class opengl_context : public graphics_context
    {
    public:
        opengl_context(GLFWwindow* window_handle);

        void init() override;
        void swap_buffers() override;

        void begin_frame() override {}
        void end_frame() override {}

    private:
        GLFWwindow* window_handle_;
    };
}
