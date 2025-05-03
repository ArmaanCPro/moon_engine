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
        m_command_list = context->get_command_list();
        ID3D12Device14* device = context->get_device().Get();
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

    directx_vertex_buffer::directx_vertex_buffer(const float* vertices, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        directx_context* context = (directx_context*)application::get().get_window().get_context();
        m_command_list = context->get_command_list();
        ID3D12Device14* device = context->get_device().Get();
        m_device = device;

        // GPU (default) heap
        D3D12_HEAP_PROPERTIES gpu_heap_properties = {};
        gpu_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
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
        upload_range.End = 0;
        if (FAILED(upload_buffer->Map(0, &upload_range, &mapped_data)))
        {
            MOON_CORE_ASSERT(false, "Failed to map upload buffer!");
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

    directx_vertex_buffer::~directx_vertex_buffer()
    {
        m_buffer.Reset();
        m_device = nullptr;
        m_command_list = nullptr;
    }

    void directx_vertex_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        m_command_list->IASetVertexBuffers(0, 1, &m_view);
    }

    void directx_vertex_buffer::set_data(const void* data, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        // Validate buffer sizes
        if (!m_buffer || size > m_view.SizeInBytes)
        {
            MOON_CORE_ASSERT(false, "Invalid size or uninitialized buffer in set_data!");
            return;
        }

        // Get the resource description to verify memory type
        D3D12_RESOURCE_DESC desc = m_buffer->GetDesc();

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

            // Create a temporary upload buffer and use a command list to copy data
            D3D12_HEAP_PROPERTIES uploadHeapProps = {};
            uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

            ComPtr<ID3D12Resource> uploadBuffer;
            D3D12_RESOURCE_DESC uploadDesc = {};
            uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            uploadDesc.Width = size;
            uploadDesc.Height = 1;
            uploadDesc.DepthOrArraySize = 1;
            uploadDesc.MipLevels = 1;
            uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
            uploadDesc.SampleDesc.Count = 1;
            uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            if (FAILED(m_device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadBuffer))))
            {
                MOON_CORE_ERROR("Failed to create upload buffer");
                return;
            }

            // Map the upload buffer (this should succeed since it's an upload heap)
            void* mappedData = nullptr;
            D3D12_RANGE readRange = { 0, 0 }; // We're not reading

            if (FAILED(uploadBuffer->Map(0, &readRange, &mappedData)))
            {
                MOON_CORE_ERROR("Failed to map upload buffer");
                return;
            }

            memcpy(mappedData, data, size);
            uploadBuffer->Unmap(0, nullptr);

            // Need a fresh command list for this operation
            auto* context = (directx_context*)application::get().get_window().get_context();
            ID3D12GraphicsCommandList10* cmdList = context->begin_resource_upload();

            cmdList->CopyBufferRegion(m_buffer.Get(), 0, uploadBuffer.Get(), 0, size);

            context->end_resource_upload();
            return;
        }

        // Map the buffer for writing
        void* mapped_data = nullptr;
        D3D12_RANGE readRange = { 0, 0 }; // Empty range means we won't read

        if (FAILED(m_buffer->Map(0, &readRange, &mapped_data)))
        {
            // Print detailed error information
            HRESULT hr = m_buffer->Map(0, &readRange, &mapped_data);
            MOON_CORE_ERROR("Failed to map upload buffer! HRESULT: 0x{0:X}", hr);

            // Debug help information
            switch (hr) {
                case E_INVALIDARG:
                    MOON_CORE_ERROR("Map failed: Invalid arguments");
                    break;
                case E_OUTOFMEMORY:
                    MOON_CORE_ERROR("Map failed: Out of memory");
                    break;
                case DXGI_ERROR_DEVICE_REMOVED:
                    MOON_CORE_ERROR("Map failed: Device removed");
                    break;
                case DXGI_ERROR_DEVICE_RESET:
                    MOON_CORE_ERROR("Map failed: Device reset");
                    break;
                default:
                    MOON_CORE_ERROR("Map failed: Unknown error");
                    break;
            }
            return;
        }

        // Copy the data to the mapped region
        memcpy(mapped_data, data, size);

        // Unmap with nullptr since we don't care about the written range
        m_buffer->Unmap(0, nullptr);
    }

    // ////////////////////////////////////////////////
    // INDEX BUFFER ///////////////////////////////////

    directx_index_buffer::directx_index_buffer(const uint32_t* indices, uint32_t count)
        :
        m_count(count)
    {
        uint32_t size = count * sizeof(uint32_t);

        directx_context* context = (directx_context*)application::get().get_window().get_context();
        m_command_list = context->get_command_list();
        ID3D12Device14* device = context->get_device().Get();

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

    directx_index_buffer::~directx_index_buffer()
    {
        m_buffer.Reset();
        m_command_list = nullptr;
    }

    void directx_index_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();
        m_command_list->IASetIndexBuffer(&m_view);
    }
}
