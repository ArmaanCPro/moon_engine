#pragma once

#include "moon/renderer/renderer_api.h"

namespace moon
{
    class opengl_renderer_api : public renderer_api
    {
    public:
        opengl_renderer_api() = default;
        ~opengl_renderer_api() override = default;

        void init() override;
        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        void clear(const glm::vec4& color) override;

        void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count) override;
    };
}
