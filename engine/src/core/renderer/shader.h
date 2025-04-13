#pragma once

#include "core/core.h"

#include <string_view>
#include <cstdint>

#include <glm/glm.hpp>

namespace moon
{
    class MOON_API shader
    {
    public:
        shader(std::string_view vertex_src, std::string_view fragment_src);
        ~shader();

        void bind() const;
        void unbind() const;

        void upload_uniform_float4(std::string_view name, const glm::vec4& vector);
        void upload_uniform_mat4(std::string_view name, const glm::mat4& matrix);

    private:
        uint32_t renderer_id_{0};
    };
}
