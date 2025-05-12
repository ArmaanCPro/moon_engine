#pragma once

#include "d3d12_context.h"
#include "moon/renderer/renderer_api.h"

namespace moon
{
    class d3d12_renderer_api : public renderer_api
    {
    public:
        d3d12_renderer_api() = default;
        ~d3d12_renderer_api() override = default;

        void init() override;
        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        void set_clear_color(const glm::vec4& color) override;
        void clear() override;

        void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count) override;

    private:
        // TEMPORARY
        d3d12_context* context_;
    };
}
