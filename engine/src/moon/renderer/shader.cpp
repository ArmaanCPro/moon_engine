#include "moonpch.h"
#include "shader.h"

#include "renderer.h"
#include "platform/opengl/opengl_shader.h"

namespace moon
{
    ref<shader> shader::create(std::string_view file_path)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return std::make_shared<opengl_shader>(file_path);
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<shader> shader::create(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return std::make_shared<opengl_shader>(name, vertex_src, fragment_src);
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    void shader_library::add(std::string_view name, const ref<shader>& shader)
    {
        MOON_CORE_ASSERT_MSG(!exists(name), "Shader already exists!");
        shaders_[name.data()] = shader;
    }

    void shader_library::add(const ref<shader>& shader)
    {
        auto name = shader->get_name();
        add(name, shader);
    }

    ref<shader> shader_library::load(std::string_view file_path)
    {
        auto shader = shader::create(file_path);
        add(shader);
        return shader;
    }

    ref<shader> shader_library::load(std::string_view name, std::string_view filepath)
    {
        auto shader = shader::create(filepath);
        add(name, shader);
        return shader;
    }

    ref<shader> shader_library::get(std::string_view name)
    {
        MOON_CORE_ASSERT_MSG(exists(name), "Shader does not exist!");
        return shaders_[name.data()];
    }

    bool shader_library::exists(std::string_view name)
    {
        return shaders_.contains(name.data());
    }
}
