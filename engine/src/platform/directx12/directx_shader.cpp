#include "moonpch.h"
#include "directx_shader.h"

namespace moon
{
    directx_shader::directx_shader(ShaderType type, std::string_view filepath)
    {
        MOON_PROFILE_FUNCTION();

        m_type = type;

        m_data = read_file(filepath);
        MOON_CORE_ASSERT(m_data.size(), "Failed to read shader file!");

        // Extract the name from the filepath
        auto last_slash = filepath.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = filepath.rfind('.');

        auto count = last_dot == std::string::npos ? filepath.size() - last_slash : last_dot - last_slash;
        m_name = filepath.substr(last_slash, count);
    }

    directx_shader::directx_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
    {
        MOON_PROFILE_FUNCTION();

        // TODO: support this in future
        MOON_CORE_ASSERT(false, "DirectX does not support vertex and fragment shaders in one file!");
        // m_name = name;
        //m_data = vertex_src + fragment_src;
    }

    void directx_shader::bind() const
    {}

    void directx_shader::unbind() const
    {}

    void directx_shader::set_int(std::string_view name, int value)
    {}

    void directx_shader::set_int_array(std::string_view name, int* values, uint32_t count)
    {}

    void directx_shader::set_float(std::string_view name, float value)
    {}

    void directx_shader::set_float2(std::string_view name, const glm::vec2& value)
    {}

    void directx_shader::set_float3(std::string_view name, const glm::vec3& value)
    {}

    void directx_shader::set_float4(std::string_view name, const glm::vec4& value)
    {}

    void directx_shader::set_mat4(std::string_view name, const glm::mat4& value)
    {}

    std::string directx_shader::read_file(std::string_view filepath)
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
