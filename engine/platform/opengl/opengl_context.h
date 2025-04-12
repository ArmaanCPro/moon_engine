#pragma once

#include "core/core.h"
#include "core/graphics_context.h"

struct GLFWwindow;

namespace moon
{
    class opengl_context : public graphics_context
    {
    public:
        opengl_context(GLFWwindow* window_handle);

        void init() override;
        void swap_buffers() override;

    private:
        GLFWwindow* window_handle_;
    };
}
