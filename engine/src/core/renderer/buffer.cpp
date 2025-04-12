#include "moonpch.h"

#include "buffer.h"
#include "renderer.h"
#include "opengl/opengl_buffer.h"

namespace moon
{
    vertex_buffer* vertex_buffer::create(const float* vertices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case RendererAPI::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case RendererAPI::OpenGL:
            return new opengl_vertex_buffer(vertices, size);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    index_buffer* index_buffer::create(const unsigned int* indices, uint32_t size)
    {
        switch (renderer::get_api())
        {
        case RendererAPI::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case RendererAPI::OpenGL:
            return new opengl_index_buffer(indices, size);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
