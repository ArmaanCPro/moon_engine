#include "moonpch.h"

#include "buffer.h"
#include "renderer.h"
#include "platform/opengl/opengl_buffer.h"

namespace moon
{
    ref<vertex_buffer> vertex_buffer::create(const float* vertices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return std::make_shared<opengl_vertex_buffer>(vertices, size);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<index_buffer> index_buffer::create(const unsigned int* indices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return std::make_shared<opengl_index_buffer>(indices, size);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
