#pragma once

#include "moon/core/core.h"

#include <string_view>
#include <unordered_map>
#include <glm/glm.hpp>

namespace moon
{
    enum class ShaderType
    {
        Unknown = 0,
        Vertex,
        Fragment,
        VertexAndFragment,
        RootSignature
    };

    class MOON_API shader
    {
    public:
        virtual ~shader() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual std::string_view get_data() = 0;
        virtual std::string_view get_name() = 0;
        ShaderType get_type() const { return m_type; }

        virtual void set_int(std::string_view name, int value) = 0;
        virtual void set_int_array(std::string_view name, int* values, uint32_t count) = 0;
        virtual void set_float(std::string_view name, float value) = 0;
        virtual void set_float2(std::string_view name, const glm::vec2& value) = 0;
        virtual void set_float3(std::string_view name, const glm::vec3& value) = 0;
        virtual void set_float4(std::string_view name, const glm::vec4& value) = 0;
        virtual void set_mat4(std::string_view name, const glm::mat4& value) = 0;

        static ref<shader> create(ShaderType type, std::string_view file_path);
        static ref<shader> create(std::string_view vertex_path, std::string_view fragment_path);
        static ref<shader> create(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);

    protected:
        ShaderType m_type = ShaderType::Unknown;
    };
}
