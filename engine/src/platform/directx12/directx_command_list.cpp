#include "moonpch.h"
#include "directx_command_list.h"

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
    {}

    void directx_command_list::begin()
    {}

    void directx_command_list::end()
    {}

    void directx_command_list::submit()
    {}

    void directx_command_list::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {
        m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
    }

    void directx_command_list::dispatch()
    {

    }
}
