#include "moonpch.h"
#include "handle.h"

#include "graphics_context.h"

namespace moon
{
    void destroy(graphics_context* ctx, compute_pipeline_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, render_pipeline_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, raytracing_pipeline_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, shader_module_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, sampler_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, buffer_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, texture_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, query_pool_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }

    void destroy(graphics_context* ctx, accel_struct_handle hdl)
    {
        if (ctx)
            ctx->destroy(hdl);
    }
}
