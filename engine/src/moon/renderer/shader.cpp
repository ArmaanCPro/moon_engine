#include "moonpch.h"
#include "shader.h"

#include "renderer.h"
#include "platform/directx12/directx_shader.h"
#include "platform/opengl/opengl_shader.h"

namespace moon
{
    ref<shader> shader::create(ShaderType type, std::string_view file_path)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_shader>(type, file_path);
        case renderer_api::API::DirectX:
            return create_ref<directx_shader>(type, file_path);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<shader> shader::create(std::string_view vertex_path, std::string_view fragment_path)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_shader>(vertex_path, fragment_path);
        case renderer_api::API::DirectX:
            return create_ref<directx_shader>(vertex_path, fragment_path);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<shader> shader::create(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_shader>(name, vertex_src, fragment_src);
        case renderer_api::API::DirectX:
            return create_ref<directx_shader>(name, vertex_src, fragment_src);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
