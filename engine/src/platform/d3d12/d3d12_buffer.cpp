#include "moonpch.h"
#include "d3d12_buffer.h"

#include "d3d12_context.h"
#include "core/application.h"

namespace moon
{
    // ////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////

    d3d12_vertex_buffer::d3d12_vertex_buffer(uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        d3d12_context* context = (d3d12_context*)application::get().get_context();
        ID3D12Device14* device = context->get_native_device().Get();
        m_device = device;

        // Using UPLOAD heap directly
        D3D12_HEAP_PROPERTIES upload_heap_properties = {};
        upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Alignment = 0;
        resource_desc.Width = size;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        resource_desc.SampleDesc.Count = 1;
        resource_desc.SampleDesc.Quality = 0;
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        if (FAILED(device->CreateCommittedResource(
            &upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, // UPLOAD heap is always in GENERIC_READ
            nullptr,
            IID_PPV_ARGS(&m_buffer)
        )))
        {
            MOON_CORE_ERROR("Failed to create vertex buffer!");
        }

        // Set the vertex buffer view
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.StrideInBytes = sizeof(float) * 5;
    }

    d3d12_vertex_buffer::d3d12_vertex_buffer(const float* vertices, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        d3d12_context* context = (d3d12_context*)application::get().get_context();
        ID3D12Device14* device = context->get_native_device().Get();
        m_device = device;

        command_list* command_list = context->get_command_list(context->get_current_buffer_index());

        // GPU (default) heap
        D3D12_HEAP_PROPERTIES gpu_heap_properties = {};
        gpu_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        gpu_heap_properties.CreationNodeMask = 0;
        gpu_heap_properties.VisibleNodeMask = 0;

        CD3DX12_RESOURCE_DESC gpu_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        if (FAILED(device->CreateCommittedResource(
            &gpu_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &gpu_resource_desc,
            D3D12_RESOURCE_STATE_COPY_DEST, // Prepare the resource for copy
            nullptr,
            IID_PPV_ARGS(&m_buffer) // This is the DEFAULT heap buffer
        )))
        {
            MOON_CORE_ERROR("Failed to create vertex buffer!");
        }

        command_list->upload_data(vertices, size, m_buffer.Get(), 0, 0);

        // Transition the GPU buffer to a generic read state after copy
        command_list->transition_resource(m_buffer.Get(), ResourceState::CopyDest, ResourceState::GenericRead);

        // Set buffer view properties
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.StrideInBytes = sizeof(float) * 5;
    }

    void d3d12_vertex_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        auto* context = (d3d12_context*)application::get().get_context();
        context->get_command_list(context->get_current_buffer_index())->bind_vertex_buffer((void*)&m_view, 0, 0);
    }

    void d3d12_vertex_buffer::set_data(const void* data, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        // Validate buffer sizes
        if (!m_buffer || size > m_view.SizeInBytes)
        {
            MOON_CORE_ASSERT(false, "Invalid size or uninitialized buffer in set_data!");
            return;
        }

        // Get heap properties
        D3D12_HEAP_PROPERTIES heapProps;
        D3D12_HEAP_FLAGS heapFlags;
        if (FAILED(m_buffer->GetHeapProperties(&heapProps, &heapFlags)))
        {
            MOON_CORE_ASSERT(false, "Failed to get heap properties!");
            return;
        }

        // Verify this is an upload heap (required for mapping)
        if (heapProps.Type != D3D12_HEAP_TYPE_UPLOAD)
        {
            MOON_CORE_ERROR("Cannot map buffer that is not on UPLOAD heap. Current heap type: {0}",
                            (int)heapProps.Type);

            auto* context = (d3d12_context*)application::get().get_context();
            command_list* cmd = context->get_command_list(context->get_current_buffer_index());
            cmd->upload_data(data, size, m_buffer.Get(), 0, 0);

            return;
        }

        // Map the buffer for writing
        void* mapped_data = nullptr;
        D3D12_RANGE readRange = { 0, 0 }; // Empty range means we won't read

        if (FAILED(m_buffer->Map(0, &readRange, &mapped_data)))
        {
            MOON_CORE_ASSERT(false, "Failed to map buffer!");
        }

        // Copy the data to the mapped region
        memcpy(mapped_data, data, size);

        // Unmap with nullptr since we don't care about the written range
        m_buffer->Unmap(0, nullptr);
    }

    // ////////////////////////////////////////////////
    // INDEX BUFFER ///////////////////////////////////

    d3d12_index_buffer::d3d12_index_buffer(const uint32_t* indices, uint32_t count)
        :
        m_count(count)
    {
        uint32_t size = count * sizeof(uint32_t);

        d3d12_context* context = (d3d12_context*)application::get().get_context();
        ID3D12Device14* device = context->get_native_device().Get();

        // create buffer in gpu memory
        D3D12_HEAP_PROPERTIES heap_properties = {};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resource_desc.Width = size;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        resource_desc.SampleDesc.Count = 1;
        resource_desc.SampleDesc.Quality = 0;
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        if (FAILED(device->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_buffer)
        )))
        {
            MOON_CORE_ERROR("Failed to create index buffer!");
        }

        // copy indices to GPU upload heap
        void* mapped_data = nullptr;
        D3D12_RANGE upload_range = {};
        upload_range.Begin = 0;
        upload_range.End = size;
        if (FAILED(m_buffer->Map(0, &upload_range, &mapped_data)))
        {
            MOON_CORE_ASSERT(false, "Failed to map index buffer!");
        }
        memcpy(mapped_data, indices, size);
        m_buffer->Unmap(0, nullptr);

        // create index buffer view
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.Format = DXGI_FORMAT_R32_UINT; // 32-bit indices
    }

    void d3d12_index_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        auto* context = (d3d12_context*)application::get().get_context();
        auto* command_list = context->get_command_list(context->get_current_buffer_index());
        command_list->bind_vertex_buffer((void*)&m_view);
    }
}
