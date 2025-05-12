#include "framebuffer.h"

#include "renderer.h"
#include "renderer_api.h"
#include "platform/d3d12/d3d12_framebuffer.h"
#include "platform/opengl/opengl_framebuffer.h"

namespace moon
{
    ref<framebuffer> framebuffer::create(const framebuffer_spec& spec)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            return create_ref<opengl_framebuffer>(spec);
        case renderer_api::API::DirectX:
            return create_ref<d3d12_framebuffer>(spec);
        }

        return nullptr;
    }
}
