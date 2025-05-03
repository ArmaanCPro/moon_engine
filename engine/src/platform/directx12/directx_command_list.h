#pragma once

#include "moon/renderer/command_list.h"
#include "directx.h"

namespace moon
{
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

        void transition_resource(void* resource, ResourceState before, ResourceState after) override;
        void set_render_target(void* target_descriptor, void* depth_stencil_desc = nullptr) override;

        void* get_native_handle() override { return m_command_list.Get(); }

    private:
        ComPtr<ID3D12GraphicsCommandList10> m_command_list;
        ID3D12Device* m_device;
        ID3D12CommandAllocator* m_allocator;

        bool m_open = false;
    };
}
