#pragma once

#include "renderer_api.h"
#include "moon/core/core.h"

#include <glm/glm.hpp>

namespace moon
{
    /// Note that render_command class is only for passing commands to the underlying renderer(_api).
    /// All low-level logic occurs in renderer_api. This is very high-level.
    class MOON_API render_command
    {
    public:
        inline static void init()
        {
            s_renderer_api_->init();
        }
        inline static void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            s_renderer_api_->set_viewport(x, y, width, height);
        }
        inline static void set_clear_color(const glm::vec4& color)
        {
            s_renderer_api_->set_clear_color(color);
        }
        inline static void clear()
        {
            s_renderer_api_->clear();
        }

        inline static void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count = 0)
        {
            s_renderer_api_->draw_indexed(vertex_array, index_count);
        }
    private:
        static renderer_api* s_renderer_api_;
    };
}
