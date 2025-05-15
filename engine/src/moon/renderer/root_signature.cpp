#include "moonpch.h"
#include "root_signature.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_root_signature.h"

namespace moon
{
    ref<root_signature_desc> root_signature_desc::create(const binding_layout& layout)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            // TODO: OpenGL rootsig wrapper
            MOON_CORE_ASSERT(false, "OpenGL does not support root signatures");
            return nullptr;
        case renderer_api::API::DirectX:
            return create_ref<d3d12_root_signature_desc>(layout);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    ref<root_signature> root_signature::create(const root_signature_desc& desc)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            MOON_CORE_ASSERT(false, "OpenGL does not support root signatures");
            return nullptr;
        case renderer_api::API::DirectX:
            return create_ref<d3d12_root_signature>(desc);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
