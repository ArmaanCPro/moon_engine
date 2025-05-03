#pragma once

#include "moon/renderer/command_list.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace moon
{
    class opengl_command_list : public command_list
    {
    public:
        opengl_command_list() = default;
        ~opengl_command_list() override = default;

        void reset() override {}    // no-op
        void begin() override {}    // no-op
        void end() override {}      // no-op
        void submit() override {}   // no-op

        void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count) override
        {
            uint32_t count = index_count ? index_count : vertex_array->get_index_buffer()->get_count();
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        }

        void dispatch() override
        {
            // glDispatchCompute, etc
        }
    };
}
