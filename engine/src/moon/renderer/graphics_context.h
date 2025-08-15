#pragma once

#include "device.h"
#include "moon/core/core.h"

#include "handle.h"

namespace moon
{
    class MOON_API graphics_context
    {
    public:
        virtual ~graphics_context() = default;

        virtual void init() = 0;
        virtual void swap_buffers() = 0;

        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;

        virtual device& get_device() = 0;

        // handle destruction
        virtual void destroy(compute_pipeline_handle hdl) = 0;
        virtual void destroy(render_pipeline_handle hdl) = 0;
        virtual void destroy(raytracing_pipeline_handle hdl) = 0;
        virtual void destroy(shader_module_handle hdl) = 0;
        virtual void destroy(sampler_handle hdl) = 0;
        virtual void destroy(buffer_handle hdl) = 0;
        virtual void destroy(texture_handle hdl) = 0;
        virtual void destroy(query_pool_handle hdl) = 0;
        virtual void destroy(accel_struct_handle hdl) = 0;
    };
}
