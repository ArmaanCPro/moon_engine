#include "moonpch.h"
#include "vertex_array.h"

#include "renderer.h"
#include "opengl/opengl_vertex_array.h"

namespace moon
{

    vertex_array* vertex_array::create()
    {
        switch (renderer::get_api())
        {
        case RendererAPI::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case RendererAPI::OpenGL:
            return new opengl_vertex_array();
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
