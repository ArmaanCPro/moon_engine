#pragma once

#include "core/renderer/renderer_api.h"

namespace moon
{
    class opengl_renderer_api : public renderer_api
    {
    public:
        opengl_renderer_api() = default;
        ~opengl_renderer_api() override = default;

        void set_clear_color(const glm::vec4& color) override;
        void clear() override;

        void draw_indexed(const ref<vertex_array>& vertex_array) override;
    };
}
