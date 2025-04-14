#pragma once

#include "moon/renderer/shader.h"

#include <string_view>
#include <cstdint>
#include <glad/glad.h>

namespace moon
{
    class opengl_shader : public shader
    {
    public:
        explicit opengl_shader(std::string_view filepath);
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
        std::string read_file(std::string_view filepath);
        std::unordered_map<GLenum, std::string> preprocess(const std::string& source);
        void compile(const std::unordered_map<GLenum, std::string>& shader_sources);
    private:
        uint32_t renderer_id_{0};
    };
}
