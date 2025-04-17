#pragma once

#include "moon/core/core.h"
#include "moon/renderer/shader.h"
#include "renderer/camera.h"
#include "renderer/camera.h"
#include "renderer/camera.h"
#include "renderer/camera.h"
#include "renderer/camera.h"
#include "renderer/camera.h"

#include <string_view>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace moon
{
    // NOTE temporarily MOON_API; normally it isn't exported into DLL but since we have some jank code in sandbox it is necessary
    class MOON_API opengl_shader : public shader
    {
    public:
        explicit opengl_shader(std::string_view filepath);
        opengl_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);
        ~opengl_shader() override;

        void bind() const override;
        void unbind() const override;

        void set_int(std::string_view name, int value) override;
        void set_int_array(std::string_view name, int* values, uint32_t count) override;
        void set_float(std::string_view name, float value) override;
        void set_float2(std::string_view name, const glm::vec2& value) override;
        void set_float3(::std::string_view name, const glm::vec3& value) override;
        void set_float4(::std::string_view name, const glm::vec4& value) override;
        void set_mat4(::std::string_view name, const glm::mat4& value) override;

        std::string_view get_name() override { return name_; }

        void upload_uniform_int(std::string_view name, int value);
        void upload_uniform_int_array(std::string_view name, int* values, uint32_t count);
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
        std::string name_;
    };
}
