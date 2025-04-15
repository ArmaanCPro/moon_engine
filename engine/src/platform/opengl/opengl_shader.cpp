#include "moonpch.h"
#include "opengl_shader.h"

#include "renderer/camera.h"
#include "renderer/camera.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <cmrc/cmrc.hpp>
CMRC_DECLARE(shaders);

namespace moon
{
    static GLenum shader_type_from_string(std::string_view type)
    {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        else if (type == "fragment"|| type == "pixel")
            return GL_FRAGMENT_SHADER;

        MOON_CORE_ASSERT(false, "Unknown shader type! {0}", type);
        return 0;
    }

    opengl_shader::opengl_shader(std::string_view filepath)
    {
        std::string shader_source = read_file(filepath);
        auto shader_sources = preprocess(shader_source);
        compile(shader_sources);

        // Extract the name from the filepath
        auto last_slash = filepath.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = filepath.rfind('.');

        auto count = last_dot == std::string::npos ? filepath.size() - last_slash : last_dot - last_slash;
        name_ = filepath.substr(last_slash, count);
    }

    opengl_shader::opengl_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
        :
        name_(name)
    {
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = std::string(vertex_src);
        sources[GL_FRAGMENT_SHADER] = std::string(fragment_src);
        compile(sources);
    }

    opengl_shader::~opengl_shader()
    {
        glDeleteProgram(renderer_id_);
    }

    std::string opengl_shader::read_file(std::string_view filepath)
    {
        std::string result;

        try
        {
            auto fs = cmrc::shaders::get_filesystem();
            if (fs.exists(filepath.data()))
            {
                auto file = fs.open(filepath.data());
                result.resize(file.size());
                std::copy(file.begin(), file.end(), result.begin());
                MOON_CORE_INFO("Loading embedded shader: {0}", filepath);
            }
            else
            {
                // Fallback to filesystem loading
                std::ifstream in(filepath.data(), std::ios::in | std::ios::binary);
                in.seekg(0, std::ios::end);
                result.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&result[0], result.size());
                MOON_CORE_INFO("Loading file system shader: {0}", filepath);
            }
        }
        catch (const std::exception& e)
        {
            // Fallback to filesystem loading
            MOON_CORE_WARN("Failed to file {0} load from embedded resources: {1}, trying filesystem", filepath, e.what());
            // Fallback to filesystem loading
            std::ifstream in(filepath.data(), std::ios::in | std::ios::binary);
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
        }

        return result;
    }

    std::unordered_map<GLenum, std::string> opengl_shader::preprocess(const std::string& source)
    {
        std::unordered_map<GLenum, std::string> result;

        const char* type_token = "#type";
        size_t type_token_length = strlen(type_token);
        size_t pos = source.find(type_token, 0);
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of('\n', pos);
            MOON_CORE_ASSERT(eol != std::string::npos, "Syntax error");
            size_t begin = pos + type_token_length + 1;
            std::string type = source.substr(begin, eol - begin);
            MOON_CORE_ASSERT(shader_type_from_string(type), "Invalid shader type specified");

            size_t next_line_begin = source.find_first_not_of('\n', eol);
            pos = source.find(type_token, next_line_begin);
            result[(GLenum)shader_type_from_string(type)] =
                source.substr(next_line_begin, pos - (next_line_begin == std::string::npos ? source.size() - 1 : next_line_begin));
        }

        return result;
    }

    void opengl_shader::compile(const std::unordered_map<GLenum, std::string>& shader_sources)
    {
        GLuint program = glCreateProgram();
        MOON_CORE_ASSERT(shader_sources.size() <= 2, "Only 2 shaders are supported!");
        std::array<GLuint, 2> gl_shader_ids {};
        int gl_shader_id_index = 0;

        for (auto&& [type, source] : shader_sources)
        {
            GLuint shader = glCreateShader(type);
            const GLchar* source_cstr = source.data();
            glShaderSource(shader, 1, &source_cstr, nullptr);
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);
                // delete previously compiled shaders
                for (auto id : gl_shader_ids)
                    glDeleteShader(id);

                MOON_CORE_ERROR("Compilation of shader type {0} failed: {1}", type, infoLog.data());
                MOON_CORE_ASSERT(false, "Shader compilation failed!");
                break;
            }

            glAttachShader(program, shader);

            gl_shader_ids[gl_shader_id_index++] = shader;
        }

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);
            // delete all shaders
            for (auto id : gl_shader_ids)
                glDeleteShader(id);

            MOON_CORE_ERROR("Shader program link failed: {0}", infoLog.data());
            MOON_CORE_ASSERT(false, "Shader program link failed!");
            return;
        }

        for (auto id : gl_shader_ids)
            glDetachShader(program, id);

        renderer_id_ = program;
    }

    void opengl_shader::bind() const
    {
        glUseProgram(renderer_id_);
    }

    void opengl_shader::unbind() const
    {
        glUseProgram(0);
    }

    void opengl_shader::set_float3(::std::string_view name, const glm::vec3& value)
    {
        upload_uniform_float3(name, value);
    }

    void opengl_shader::set_float4(std::string_view name, const glm::vec4& value)
    {
        upload_uniform_float4(name, value);
    }

    void opengl_shader::set_mat4(std::string_view name, const glm::mat4& value)
    {
        upload_uniform_mat4(name, value);
    }

    void opengl_shader::upload_uniform_int(std::string_view name, int value)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniform1i(location, value);
    }

    void opengl_shader::upload_uniform_float(std::string_view name, float value)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniform1f(location, value);
    }

    void opengl_shader::upload_uniform_float2(std::string_view name, const glm::vec2& vector)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniform2f(location, vector.x, vector.y);
    }

    void opengl_shader::upload_uniform_float3(std::string_view name, const glm::vec3& vector)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniform3f(location, vector.x, vector.y, vector.z);
    }

    void opengl_shader::upload_uniform_float4(std::string_view name, const glm::vec4& vector)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
    }

    void opengl_shader::upload_uniform_mat3(std::string_view name, const glm::mat3& matrix)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void opengl_shader::upload_uniform_mat4(std::string_view name, const glm::mat4& matrix)
    {
        GLint location = glGetUniformLocation(renderer_id_, name.data());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}
