#include "moonpch.h"
#include "shader.h"

#include "renderer.h"
#include "opengl/opengl_shader.h"

namespace moon
{
    shader* shader::create(std::string_view vertex_src, std::string_view fragment_src)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return new opengl_shader(vertex_src, fragment_src);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
