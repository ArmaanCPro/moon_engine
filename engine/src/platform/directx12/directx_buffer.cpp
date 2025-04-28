#include "moonpch.h"
#include "directx_buffer.h"

#include "directx_context.h"
#include "core/application.h"

namespace moon
{
    // ////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////

    directx_vertex_buffer::directx_vertex_buffer(uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        directx_context* context = (directx_context*)application::get().get_window().get_context();
        m_command_list = context->init_command_list();
        ID3D12Device14* device = context->get_device().Get();
        m_device = device;

        // GPU (DEFAULT) heap vertex buffer
        D3D12_HEAP_PROPERTIES default_heap_properties = {};
        default_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        default_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        default_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        default_heap_properties.CreationNodeMask = 0;
        default_heap_properties.VisibleNodeMask = 0;

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
            &default_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            D3D12_RESOURCE_STATE_COPY_DEST, // Initial state must allow copying from an upload heap
            nullptr,
            IID_PPV_ARGS(&m_buffer) // buffer located in GPU memory
        )))
        {
            MOON_CORE_ERROR("Failed to create vertex buffer in DEFAULT heap!");
        }

        // Create the intermediate UPLOAD heap buffer for staging updates
        D3D12_HEAP_PROPERTIES upload_heap_properties = {};
        upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

        if (FAILED(device->CreateCommittedResource(
            &upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, // Always in GENERIC_READ for UPLOAD resources
            nullptr,
            IID_PPV_ARGS(&m_buffer) // This is the buffer located in system memory
        )))
        {
            MOON_CORE_ERROR("Failed to create vertex buffer in UPLOAD heap!");
        }

        // Set the vertex buffer view (for the GPU buffer)
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.StrideInBytes = sizeof(float) * 5; // For now, this is 5 floats per vertex
    }

    directx_vertex_buffer::directx_vertex_buffer(const float* vertices, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        directx_context* context = (directx_context*)application::get().get_window().get_context();
        m_command_list = context->init_command_list();
        ID3D12Device14* device = context->get_device().Get();
        m_device = device;

        // GPU (default) heap
        D3D12_HEAP_PROPERTIES gpu_heap_properties = {};
        gpu_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        gpu_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        gpu_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        gpu_heap_properties.CreationNodeMask = 0;
        gpu_heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC gpu_resource_desc = {};
        gpu_resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        gpu_resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        gpu_resource_desc.Width = size;
        gpu_resource_desc.Height = 1;
        gpu_resource_desc.DepthOrArraySize = 1;
        gpu_resource_desc.MipLevels = 1;
        gpu_resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        gpu_resource_desc.SampleDesc.Count = 1;
        gpu_resource_desc.SampleDesc.Quality = 0;
        gpu_resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        gpu_resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

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

        // Create an intermediate UPLOAD buffer
        ComPtr<ID3D12Resource> upload_buffer;
        D3D12_HEAP_PROPERTIES upload_heap_properties = {};
        upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

        if (FAILED(device->CreateCommittedResource(
            &upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &gpu_resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, // Always in GENERIC_READ for UPLOAD
            nullptr,
            IID_PPV_ARGS(&upload_buffer)
        )))
        {
            MOON_CORE_ERROR("Failed to create upload buffer!");
        }

        // Map the upload buffer and copy resource data into it
        void* mapped_data = nullptr;
        D3D12_RANGE upload_range = {};
        upload_range.Begin = 0;
        upload_range.End = size;
        if (FAILED(upload_buffer->Map(0, &upload_range, &mapped_data)))
        {
            MOON_CORE_ERROR("Failed to map upload buffer!");
        }
        memcpy(mapped_data, vertices, size);
        upload_buffer->Unmap(0, nullptr);

        // Copy data from the upload buffer to the GPU buffer
        m_command_list->CopyBufferRegion(
            m_buffer.Get(),
            0,
            upload_buffer.Get(),
            0,
            size);

        // Transition the GPU buffer to a generic read state after copy
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = m_buffer.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        m_command_list->ResourceBarrier(1, &barrier);

        // Set buffer view properties
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.StrideInBytes = sizeof(float) * 5;
    }

    void directx_vertex_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        m_command_list->IASetVertexBuffers(0, 1, &m_view);
    }

    void directx_vertex_buffer::set_data(const void* data, uint32_t size)
    {
        // Validate buffer sizes
        if (!m_buffer || size > m_view.SizeInBytes)
        {
            MOON_CORE_ERROR("Invalid size or uninitialized buffer in set_data!");
            return;
        }

        // Map the upload buffer and copy data into it
        void* mapped_data = nullptr;
        D3D12_RANGE write_range = {};
        write_range.Begin = 0;
        write_range.End = size;

        if (FAILED(m_buffer->Map(0, &write_range, &mapped_data)))
        {
            MOON_CORE_ERROR("Failed to map upload buffer!");
            return;
        }

        memcpy(mapped_data, data, size);
        m_buffer->Unmap(0, nullptr);

        // Copy data from the upload buffer to the GPU buffer
        m_command_list->CopyResource(m_buffer.Get(), m_buffer.Get());

        // Optional: Add a resource barrier to transition the GPU buffer to GENERIC_READ state (if needed)
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = m_buffer.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        m_command_list->ResourceBarrier(1, &barrier);
    }

    // ////////////////////////////////////////////////
    // INDEX BUFFER ///////////////////////////////////

    directx_index_buffer::directx_index_buffer(const uint32_t* indices, uint32_t count)
        :
        m_count(count)
    {
        uint32_t size = count * sizeof(uint32_t);

        directx_context* context = (directx_context*)application::get().get_window().get_context();
        m_command_list = context->init_command_list();
        ID3D12Device14* device = context->get_device().Get();

        // create buffer in gpu memory
        D3D12_HEAP_PROPERTIES heap_properties = {};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
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
            MOON_CORE_ERROR("Failed to map index buffer!");
        }
        memcpy(mapped_data, indices, size);
        m_buffer->Unmap(0, nullptr);

        // create index buffer view
        m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
        m_view.SizeInBytes = size;
        m_view.Format = DXGI_FORMAT_R32_UINT; // 32-bit indices
    }

    void directx_index_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        m_command_list->IASetIndexBuffer(&m_view);
    }
}
