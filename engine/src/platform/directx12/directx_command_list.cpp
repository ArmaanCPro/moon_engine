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

        m_command_list->Close();
        m_allocator->Reset();
        m_command_list->Reset(m_allocator, nullptr);
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

        // MOON_CORE_ASSERT(!m_open, "Command list is already open!");

        m_command_list->Close();
        m_allocator->Reset();
        m_command_list->Reset(m_allocator, nullptr);
        m_open = true;
    }

    void directx_command_list::begin()
    {
        MOON_PROFILE_FUNCTION();

        // MOON_CORE_ASSERT(!m_open, "Command list is already open!");

        // TODO: We are closing this here because the command list is already open in the constructor
        // and we need the command list to be open in the ctor because Renderer::Init happens in the ctor and needs the command list to be open
        m_command_list->Close();
        m_allocator->Reset();
        m_command_list->Reset(m_allocator, nullptr);
        m_open = true;
    }

    void directx_command_list::end()
    {
        MOON_PROFILE_FUNCTION();

        // MOON_CORE_ASSERT(m_open, "Command list is not open!");

        HRESULT hr = m_command_list->Close();
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to close command list");

        m_open = false;
    }

    void directx_command_list::submit()
    {
        MOON_PROFILE_FUNCTION();

        // MOON_CORE_ASSERT(m_open, "Command list is not open!");

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

    void directx_command_list::upload_data(const void* data, size_t size, void* gpu_buffer, size_t dest_offset,
        size_t src_offset)
    {
        MOON_PROFILE_FUNCTION();

        ComPtr<ID3D12Resource2> upload_buffer;
        CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        HRESULT hr = m_device->CreateCommittedResource(
            &heap_props,
            D3D12_HEAP_FLAG_NONE,
            &buffer_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_buffer)
        );
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create upload buffer!");

        void* mapped_data = nullptr;
        CD3DX12_RANGE read_range(0, 0);
        upload_buffer->Map(0, &read_range, &mapped_data);
        memcpy(mapped_data, data, size);
        upload_buffer->Unmap(0, nullptr);

        m_command_list->CopyBufferRegion(
            (ID3D12Resource*)gpu_buffer,
            dest_offset,
            upload_buffer.Get(),
            src_offset,
            size
        );
    }

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

    void directx_command_list::transition_resource(void* resource, ResourceState before, ResourceState after, size_t num_barriers)
    {
        MOON_PROFILE_FUNCTION();

        CD3DX12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition((ID3D12Resource*)resource, to_d3d_state(before), to_d3d_state(after));

        m_command_list->ResourceBarrier((UINT)num_barriers, &barrier);
    }

    void directx_command_list::set_render_target(void* target_descriptor, void* depth_stencil_desc)
    {
        MOON_PROFILE_FUNCTION();

        auto* target = (D3D12_CPU_DESCRIPTOR_HANDLE*)target_descriptor;
        auto* depth_stencil = (D3D12_CPU_DESCRIPTOR_HANDLE*)depth_stencil_desc;
        m_command_list->OMSetRenderTargets(1, target, FALSE, depth_stencil);
    }

    void directx_command_list::bind_vertex_buffer(void* vbuf_view, size_t num_views, size_t start_slot)
    {
        MOON_PROFILE_FUNCTION();

        auto* vbuf_view_ptr = (D3D12_VERTEX_BUFFER_VIEW*)vbuf_view;
        m_command_list->IASetVertexBuffers((UINT)start_slot, (UINT)num_views, vbuf_view_ptr);
    }
}
