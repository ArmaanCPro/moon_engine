#include "moonpch.h"
#include "opengl_shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    opengl_shader::opengl_shader(std::string_view vertex_src, std::string_view fragment_src)
    {
        const GLchar* vertex_src_cstr = vertex_src.data();
        const GLchar* fragment_src_cstr = fragment_src.data();

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(vertex_shader, 1, &vertex_src_cstr, nullptr);

        glCompileShader(vertex_shader);
        GLint isCompiled = 0;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertex_shader);

            MOON_CORE_ERROR("{0}", infoLog.data());
            MOON_CORE_ASSERT(false, "Vertex shader compilation failed:");
            return;
        }

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_src_cstr, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(fragment_shader);
            glDeleteShader(vertex_shader);

            MOON_CORE_ERROR("{0}", infoLog.data());
            MOON_CORE_ASSERT(false, "Fragment shader compilation failed");
            return;
        }

        renderer_id_ = glCreateProgram();
        glAttachShader(renderer_id_, vertex_shader);
        glAttachShader(renderer_id_, fragment_shader);
        glLinkProgram(renderer_id_);

        GLint isLinked = 0;
        glGetProgramiv(renderer_id_, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(renderer_id_, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(renderer_id_, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(renderer_id_);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);

            MOON_CORE_ERROR("{0}", infoLog.data());
            MOON_CORE_ASSERT(false, "Shader program link failed");
            return;
        }

        // Always detach shaders after a successful link.
        glDetachShader(renderer_id_, vertex_shader);
        glDetachShader(renderer_id_, fragment_shader);
    }

    opengl_shader::~opengl_shader()
    {
        glDeleteProgram(renderer_id_);
    }

    void opengl_shader::bind() const
    {
        glUseProgram(renderer_id_);
    }

    void opengl_shader::unbind() const
    {
        glUseProgram(0);
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
