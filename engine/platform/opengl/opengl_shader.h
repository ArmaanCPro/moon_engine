#pragma once

#include "core/renderer/shader.h"

#include <string_view>
#include <cstdint>

namespace moon
{
    class opengl_shader : public shader
    {
    public:
        opengl_shader(std::string_view vertex_src, std::string_view fragment_src);
        ~opengl_shader() override;

        void bind() const override;
        void unbind() const override;

        void upload_uniform_int(std::string_view name, int value);
        void upload_uniform_float(std::string_view name, float value);
        void upload_uniform_float2(std::string_view name, const glm::vec2& vector);
        void upload_uniform_float3(std::string_view name, const glm::vec3& vector);
        void upload_uniform_float4(std::string_view name, const glm::vec4& vector);
        void upload_uniform_mat3(std::string_view name, const glm::mat3& matrix);
        void upload_uniform_mat4(std::string_view name, const glm::mat4& matrix);

    private:
        uint32_t renderer_id_{0};
    };
}
