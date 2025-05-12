#include "moonpch.h"
#include "texture.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_texture.h"
#include "platform/opengl/opengl_texture.h"

namespace moon
{
    ref<texture2d> texture2d::create(uint32_t width, uint32_t height)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_texture2d>(width, height);
        case renderer_api::API::DirectX:
            return create_ref<d3d12_texture2d>(width, height);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<texture2d> texture2d::create(std::string_view path)
    {
        switch (renderer::get_api())
        {
            case renderer_api::API::None:
                MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
                return nullptr;
            case renderer_api::API::OpenGL:
                return create_ref<opengl_texture2d>(path);
            case renderer_api::API::DirectX:
                return create_ref<d3d12_texture2d>(path);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
