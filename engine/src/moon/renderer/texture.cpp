#include "moonpch.h"
#include "texture.h"

#include "renderer.h"
#include "core/application.h"
#include "platform/opengl/opengl_texture.h"
#include "vulkan/vk_context.h"
#include "vulkan/vk_texture.h"

namespace moon
{
    ref<texture2d> texture2d::create(uint32_t width, uint32_t height)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_texture2d>(width, height);
        case renderer_api::API::Vulkan:
            return create_ref<vulkan::vk_texture2d>(dimensions{width, height, 1},
                                                    reinterpret_cast<vulkan::vk_context&>(application::get().get_context()));
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<texture2d> texture2d::create(std::string_view path)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_texture2d>(path);
        case renderer_api::API::Vulkan:
            return create_ref<vulkan::vk_texture2d>(path,
                    static_cast<vulkan::vk_context&>(application::get().get_context()));
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
