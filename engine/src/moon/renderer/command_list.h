#pragma once

#include "moon/core/core.h"
#include "renderer_api.h"

namespace moon
{
    enum class ResourceState
    {
        Undefined = 0,
        Common,
        RenderTarget,
        Present,
        CopySource,
        CopyDest,
        GenericRead,
        FragmentShaderResource,
        ShaderResource
    };

    class MOON_API command_list
    {
    public:
        virtual ~command_list() = default;

        virtual void reset() = 0;
        virtual void begin() = 0;
        virtual void end() = 0;
        virtual void submit() = 0;

        virtual void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count = 0) = 0;
        virtual void dispatch() = 0;

        virtual void upload_data(const void* data, size_t size, void* gpu_buffer, size_t dest_offset = 0, size_t src_offset = 0) = 0;
        virtual void transition_resource(void* resource, ResourceState before, ResourceState after, size_t num_barriers = 1) = 0;
        virtual void set_render_target(void* target_descriptor, void* depth_stencil_desc = nullptr) = 0;

        virtual void bind_vertex_buffer(void* vbuf_view, size_t num_views = 1, size_t start_slot = 0) = 0;

        // TEMP
        virtual void* get_native_handle() = 0;
    };
}
