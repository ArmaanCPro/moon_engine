#include "moonpch.h"
#include "directx_command_list.h"

#include "directx_context.h"
#include "core/application.h"

namespace moon
{
    directx_command_list::directx_command_list(ID3D12Device* device, ID3D12CommandAllocator* allocator)
        :
        m_device(device),
        m_allocator(allocator)
    {
        MOON_PROFILE_FUNCTION();

        HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&m_command_list));
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create command list");
    }

    directx_command_list::~directx_command_list()
    {
        MOON_PROFILE_FUNCTION();

        m_allocator = nullptr;
        m_device = nullptr;
        m_command_list.Reset();
    }

    void directx_command_list::reset()
    {
        MOON_PROFILE_FUNCTION();

        m_allocator->Reset();
        m_command_list->Reset(m_allocator, nullptr);
        m_open = true;
    }

    void directx_command_list::begin()
    {
        MOON_PROFILE_FUNCTION();

        m_allocator->Reset();
        m_command_list->Reset(m_allocator, nullptr);
        m_open = true;
    }

    void directx_command_list::end()
    {
        MOON_PROFILE_FUNCTION();

        MOON_CORE_ASSERT(m_open, "Command list is not open!");

        HRESULT hr = m_command_list->Close();
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to close command list");

        m_open = false;
    }

    void directx_command_list::submit()
    {
        MOON_PROFILE_FUNCTION();

        MOON_CORE_ASSERT(m_open, "Command list is not open!");

        auto* context = (directx_context*)application::get().get_context();
        auto* dx_queue = context->get_command_queue().Get();
        auto* dx_fence = context->get_fence().Get();
        auto& dx_fence_value = context->get_fence_value();

        ID3D12CommandList* command_lists[] = { m_command_list.Get() };
        dx_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

        dx_queue->Signal(dx_fence, ++dx_fence_value);
    }

    void directx_command_list::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {
        MOON_PROFILE_FUNCTION();

        vertex_array->bind();

        m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
    }

    void directx_command_list::dispatch()
    {

    }

    static D3D12_RESOURCE_STATES to_d3d_state(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::RenderTarget:                 return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ResourceState::Present:                      return D3D12_RESOURCE_STATE_PRESENT;
        case ResourceState::CopySource:                   return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case ResourceState::CopyDest:                     return D3D12_RESOURCE_STATE_COPY_DEST;
        case ResourceState::FragmentShaderResource:       return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        case ResourceState::ShaderResource:               return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        default:                                          return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    void directx_command_list::transition_resource(void* resource, ResourceState before, ResourceState after)
    {
        MOON_PROFILE_FUNCTION();

        CD3DX12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition((ID3D12Resource*)resource, to_d3d_state(before), to_d3d_state(after));

        m_command_list->ResourceBarrier(1, &barrier);
    }

    void directx_command_list::set_render_target(void* target_descriptor, void* depth_stencil_desc)
    {
        MOON_PROFILE_FUNCTION();

        auto* target = (D3D12_CPU_DESCRIPTOR_HANDLE*)target_descriptor;
        auto* depth_stencil = (D3D12_CPU_DESCRIPTOR_HANDLE*)depth_stencil_desc;
        m_command_list->OMSetRenderTargets(1, target, FALSE, depth_stencil);
    }
}
