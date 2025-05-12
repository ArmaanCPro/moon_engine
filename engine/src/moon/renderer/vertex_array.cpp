#include "moonpch.h"
#include "vertex_array.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_vertex_array.h"
#include "platform/opengl/opengl_vertex_array.h"

namespace moon
{
    ref<vertex_array> vertex_array::create()
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_vertex_array>();
        case renderer_api::API::DirectX:
            return create_ref<d3d12_vertex_array>();
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
