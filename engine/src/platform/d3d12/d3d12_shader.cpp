#include "moonpch.h"
#include "d3d12_shader.h"

#include "d3d12_context.h"
#include "core/application.h"

#include "d3d12_include.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>

namespace moon
{
    d3d12_shader::d3d12_shader(ShaderType type, std::string_view filepath)
    {
        MOON_PROFILE_FUNCTION();

        m_type = type;

        m_data = read_file(filepath);
        MOON_CORE_ASSERT(!m_data.empty(), "Failed to read shader file!");

        // Extract the name from the filepath
        auto last_slash = filepath.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = filepath.rfind('.');

        auto count = last_dot == std::string::npos ? filepath.size() - last_slash : last_dot - last_slash;
        m_name = filepath.substr(last_slash, count);
    }

    d3d12_shader::d3d12_shader(std::string_view vertex_path, std::string_view fragment_path)
    {
        MOON_PROFILE_FUNCTION();

        m_type = ShaderType::VertexAndFragment;

        m_data = read_file(vertex_path) + read_file(fragment_path);
        MOON_CORE_ASSERT(!m_data.empty(), "Failed to read shader file!");

        // Extract the name from the filepath
        auto last_slash = vertex_path.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = vertex_path.rfind('.');

        auto count = last_dot == std::string::npos ? vertex_path.size() - last_slash : last_dot - last_slash;
        m_name = vertex_path.substr(last_slash, count);
    }

    d3d12_shader::d3d12_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
    {
        MOON_PROFILE_FUNCTION();

        // TODO: support this in future
        MOON_CORE_ASSERT(false, "DirectX does not support vertex and fragment shaders in one file!");
        // m_name = name;
        //m_data = vertex_src + fragment_src;
    }

    d3d12_shader::~d3d12_shader()
    {

    }

    void d3d12_shader::bind() const
    {
        MOON_PROFILE_FUNCTION();

    }

    void d3d12_shader::unbind() const
    {

    }

    void d3d12_shader::set_int(std::string_view name, int value)
    {}

    void d3d12_shader::set_int_array(std::string_view name, int* values, uint32_t count)
    {
        // no-op
    }

    void d3d12_shader::set_float(std::string_view name, float value)
    {

    }

    void d3d12_shader::set_float2(std::string_view name, const glm::vec2& value)
    {}

    void d3d12_shader::set_float3(std::string_view name, const glm::vec3& value)
    {}

    void d3d12_shader::set_float4(std::string_view name, const glm::vec4& value)
    {}

    void d3d12_shader::set_mat4(std::string_view name, const glm::mat4& value)
    {
        MOON_PROFILE_FUNCTION();


    }

    std::string d3d12_shader::read_file(std::string_view filepath)
    {
        MOON_PROFILE_FUNCTION();


        std::string result;

        std::ifstream in(filepath.data(), std::ios::binary);

        if (in)
        {
            in.seekg(0, std::ios::end);
            std::streampos size = in.tellg();
            if (size != -1)
            {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], result.size());
                in.close();
            }
            else
            {
                MOON_CORE_ERROR("Failed to read file: {0}", filepath);
            }
        }
        else
        {
            MOON_CORE_ERROR("Failed to open file: {0}", filepath);
        }

        return result;
    }
}
