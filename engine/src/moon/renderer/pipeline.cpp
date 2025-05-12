#include "moonpch.h"
#include "pipeline.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_pipeline.h"

namespace moon
{
    ref<pipeline> pipeline::create(const pipeline_spec& spec)
    {
        switch (renderer::get_api())
        {
            case renderer_api::API::None:
                MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
                return nullptr;
            case renderer_api::API::OpenGL:
                return nullptr;
                //return create_ref<opengl_pipeline>(spec);
            case renderer_api::API::DirectX:
                return create_ref<d3d12_pipeline>(spec);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
