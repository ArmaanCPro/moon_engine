#include "moonpch.h"
#include "graphics_context.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_context.h"
#include "platform/opengl/opengl_context.h"

namespace moon
{
    graphics_context* graphics_context::create(void* window_handle)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return new opengl_context((GLFWwindow*)window_handle);
        case renderer_api::API::DirectX:
            return new d3d12_context((HWND)window_handle);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
