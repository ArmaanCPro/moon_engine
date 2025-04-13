#pragma once

#include "vertex_array.h"
#include "core/core.h"
#include <glm/glm.hpp>

namespace moon
{
    class renderer_api
    {
    public:
        virtual ~renderer_api() = default;

        enum class API
        {
            None = 0,
            OpenGL = 1
        };

        virtual void set_clear_color(const glm::vec4& color) = 0;
        virtual void clear() = 0;

        virtual void draw_indexed(const ref<vertex_array>& vertex_array) = 0;

        static API get_api() { return s_API_; }
    private:
        static API s_API_;
    };

}
