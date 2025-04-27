#pragma once

#include "vertex_array.h"
#include "moon/core/core.h"
#include <glm/glm.hpp>
#include <cstdint>

namespace moon
{
    class MOON_API renderer_api
    {
    public:
        virtual ~renderer_api() = default;

        enum class API
        {
            None = 0,
            OpenGL = 1,
            DirectX = 2
        };

        virtual void init() = 0;
        virtual void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual void set_clear_color(const glm::vec4& color) = 0;
        virtual void clear() = 0;

        virtual void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count = 0) = 0;

        static API get_api() { return s_API_; }
    private:
        static API s_API_;
    };

}
