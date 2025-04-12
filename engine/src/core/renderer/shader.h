#pragma once

#include "core/core.h"

#include <string_view>
#include <cstdint>

namespace moon
{
    class MOON_API shader
    {
    public:
        shader(std::string_view vertex_src, std::string_view fragment_src);
        ~shader();

        void bind() const;
        void unbind() const;

    private:
        uint32_t renderer_id_{0};
    };
}
