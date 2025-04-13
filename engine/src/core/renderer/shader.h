#pragma once

#include "core/core.h"

#include <string_view>

#include <glm/glm.hpp>

namespace moon
{
    class MOON_API shader
    {
    public:
        virtual ~shader() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        static shader* create(std::string_view vertex_src, std::string_view fragment_src);
    };
}
