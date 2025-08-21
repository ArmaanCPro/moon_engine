#include "moonpch.h"
#include "shader.h"

#include "renderer.h"
#include "core/application.h"
#include "vulkan/vk_context.h"
#include "vulkan/vk_shader.h"

namespace moon
{
    scope<shader> shader::create(std::string_view file_path, ShaderStage stage)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::Vulkan:
            return create_scope<vulkan::vk_shader>(file_path, static_cast<vulkan::vk_context&>(application::get().get_context()), stage);
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    scope<shader> shader::create(std::string_view name, std::string_view src, ShaderStage shader_stage)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::Vulkan:
            return create_scope<vulkan::vk_shader>(src, static_cast<vulkan::vk_context&>(application::get().get_context()), shader_stage);
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
        ref<shader> shader = ref{ shader::create(file_path, ShaderStage::Vert) };
        add(shader);
        return shader;
    }

    ref<shader> shader_library::load(std::string_view name, std::string_view filepath)
    {
        ref<shader> shader = ref{ shader::create(filepath, ShaderStage::Vert) };
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
