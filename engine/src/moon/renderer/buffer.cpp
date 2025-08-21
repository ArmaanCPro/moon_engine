#include "moonpch.h"

#include "buffer.h"
#include "renderer.h"
#include "core/application.h"
#include "platform/opengl/opengl_buffer.h"
#include "vulkan/vk_buffer.h"

namespace moon
{
    scope<vertex_buffer> vertex_buffer::create(uint32_t size)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_scope<opengl_vertex_buffer>(size);
        case renderer_api::API::Vulkan:
            return create_scope<vulkan::vk_vertex_buffer>(size, reinterpret_cast<vulkan::vk_context&>(application::get().get_context()));
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    scope<vertex_buffer> vertex_buffer::create(const float* vertices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_scope<opengl_vertex_buffer>(vertices, size);
        case renderer_api::API::Vulkan:
            return create_scope<vulkan::vk_vertex_buffer>(vertices, size, reinterpret_cast<vulkan::vk_context&>(application::get().get_context()));
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<index_buffer> index_buffer::create(const unsigned int* indices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT_MSG(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return std::make_shared<opengl_index_buffer>(indices, size);
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
