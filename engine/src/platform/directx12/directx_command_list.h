#pragma once

#include "moon/renderer/command_list.h"
#include "directx.h"

namespace moon
{
    static D3D12_RESOURCE_STATES to_d3d_state(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::RenderTarget:                 return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ResourceState::Present:                      return D3D12_RESOURCE_STATE_PRESENT;
        case ResourceState::CopySource:                   return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case ResourceState::CopyDest:                     return D3D12_RESOURCE_STATE_COPY_DEST;
        case ResourceState::GenericRead:                  return D3D12_RESOURCE_STATE_GENERIC_READ;
        case ResourceState::FragmentShaderResource:       return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        case ResourceState::ShaderResource:               return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        default:                                          return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    class directx_command_list : public command_list
    {
    public:
        directx_command_list(ID3D12Device* device, ID3D12CommandAllocator* allocator);
        ~directx_command_list() override;

        void reset() override;
        // optional, same as reset
        void begin() override;
        // closes the command list
        void end() override;
        // submits to queue, and signals a fence
        void submit() override;

        void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count) override;
        void dispatch() override;

        void upload_data(const void* data, size_t size, void* gpu_buffer, size_t dest_offset, size_t src_offset) override;
        void transition_resource(void* resource, ResourceState before, ResourceState after, size_t num_barriers = 1) override;
        void set_render_target(void* target_descriptor, void* depth_stencil_desc = nullptr) override;

        void bind_vertex_buffer(void* vbuf_view, size_t num_views, size_t start_slot = 0) override;

        void* get_native_handle() override { return m_command_list.Get(); }

    private:
        ComPtr<ID3D12GraphicsCommandList10> m_command_list;
        ID3D12Device* m_device;
        ID3D12CommandAllocator* m_allocator;

        bool m_open = false;
    };
}
