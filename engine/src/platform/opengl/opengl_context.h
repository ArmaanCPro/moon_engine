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
        void shutdown() override {}
        void swap_buffers() override;

        command_list* get_command_list(uint32_t frame_index) override
        {
            // temp
            return nullptr;
        }

    private:
        GLFWwindow* window_handle_;
    };
}
