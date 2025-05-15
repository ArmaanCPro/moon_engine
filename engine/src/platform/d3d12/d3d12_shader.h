#pragma once

#include "moon/renderer/shader.h"
#include "d3d12_include.h"

namespace moon
{
    class d3d12_shader : public shader
    {
    public:
        explicit d3d12_shader(ShaderType type, std::string_view filepath);
        d3d12_shader(std::string_view vertex_path, std::string_view fragment_path);
        d3d12_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);
        ~d3d12_shader() override;

        std::string_view get_data() override { return m_data; }
        std::string_view get_name() override { return m_name; }

        void bind() const override;
        void unbind() const override;

        void set_int(std::string_view name, int value) override;
        void set_int_array(std::string_view name, int* values, uint32_t count) override;
        void set_float(std::string_view name, float value) override;
        void set_float2(std::string_view name, const glm::vec2& value) override;
        void set_float3(::std::string_view name, const glm::vec3& value) override;
        void set_float4(::std::string_view name, const glm::vec4& value) override;
        void set_mat4(::std::string_view name, const glm::mat4& value) override;

    private:
        std::string read_file(std::string_view filepath);

    private:
        std::string m_name;
        std::string m_data;
    };
}
