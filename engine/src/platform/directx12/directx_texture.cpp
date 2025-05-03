#include "moonpch.h"
#include "directx_texture.h"

#include "core/application.h"
#include "stb_image.h"

moon::directx_texture2d::directx_texture2d(uint32_t width, uint32_t height)
    :
    m_width(width),
    m_height(height)
{
    MOON_PROFILE_FUNCTION();

    m_context = (directx_context*)application::get().get_context();

    m_dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Default heap properties
    D3D12_HEAP_PROPERTIES hp_default = {};
    hp_default.Type = D3D12_HEAP_TYPE_DEFAULT;
    hp_default.CreationNodeMask = 0;
    hp_default.VisibleNodeMask = 0;

    // Resource Description
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    rd.Width = m_width;
    rd.Height = m_height;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = m_dxgi_format;
    rd.SampleDesc.Count = 1;
    rd.SampleDesc.Quality = 0;
    rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(m_context->get_device()->CreateCommittedResource(
        &hp_default,
        D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_texture_resource))))
    {
        MOON_CORE_ERROR("Failed to create texture resource!");
    }

    // Descriptor Heap for texture
    D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
    dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    dhd.NumDescriptors = 1;
    dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    dhd.NodeMask = 0;

    m_context->get_device()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_descriptor_heap));

    // SRV for texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = m_dxgi_format;
    srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.PlaneSlice = 0;
    srvd.Texture2D.ResourceMinLODClamp = 0.0f;

    m_context->get_device()->CreateShaderResourceView(m_texture_resource.Get(), &srvd, m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
}

moon::directx_texture2d::directx_texture2d(std::string_view path)
{
    MOON_PROFILE_FUNCTION();

    // TODO: Consider replacing stb_image for WIC
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 4); // force 4 channels
    MOON_CORE_ASSERT(data, "Failed to load image!");

    m_width = (uint32_t)width;
    m_height = (uint32_t)height;
    m_dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    channels = 4;

    m_context = (directx_context*)application::get().get_context();

    // Create default heap for the texture resource
    D3D12_HEAP_PROPERTIES default_heap_properties = {};
    default_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC texture_desc = {};
    texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texture_desc.Alignment = 0;
    texture_desc.Width = m_width;
    texture_desc.Height = m_height;
    texture_desc.DepthOrArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.Format = m_dxgi_format;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(m_context->get_device()->CreateCommittedResource(
        &default_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &texture_desc,
        D3D12_RESOURCE_STATE_COPY_DEST, // Initial state for uploading data
        nullptr,
        IID_PPV_ARGS(&m_texture_resource))))
    {
        MOON_CORE_ERROR("Failed to create texture resource!");
        stbi_image_free(data);
        return;
    }

    // Create an upload heap for uploading the texture data
    uint64_t required_buffer_size = GetRequiredIntermediateSize(m_texture_resource.Get(), 0, 1);

    D3D12_HEAP_PROPERTIES upload_heap_properties = {};
    upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC upload_buffer_desc = {};
    upload_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_buffer_desc.Alignment = 0;
    upload_buffer_desc.Width = required_buffer_size;
    upload_buffer_desc.Height = 1;
    upload_buffer_desc.DepthOrArraySize = 1;
    upload_buffer_desc.MipLevels = 1;
    upload_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_buffer_desc.SampleDesc.Count = 1;
    upload_buffer_desc.SampleDesc.Quality = 0;
    upload_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(m_context->get_device()->CreateCommittedResource(
        &upload_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &upload_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_upload_buffer))))
    {
        MOON_CORE_ERROR("Failed to create upload buffer!");
        stbi_image_free(data);
        return;
    }

    // Copy the texture data into the upload heap
    D3D12_SUBRESOURCE_DATA texture_data = {};
    texture_data.pData = data;
    texture_data.RowPitch = m_width * channels; // Assuming tightly packed data
    texture_data.SlicePitch = texture_data.RowPitch * m_height;

    ID3D12GraphicsCommandList* command_list = m_context->get_native_command_list();
    // UpdateSubresources(command_list, m_texture_resource.Get(), m_upload_buffer.Get(), 0, 0, 1, &texture_data);

    // Transition the texture to a shader-readable state
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_texture_resource.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    command_list->ResourceBarrier(1, &barrier);

    // m_context->execute_command_lists();

    // Create a shader resource view (SRV) for the texture
    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.NumDescriptors = 1;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(m_context->get_device()->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&m_descriptor_heap))))
    {
        MOON_CORE_ERROR("Failed to create descriptor heap!");
        stbi_image_free(data);
        return;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = m_dxgi_format;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    m_context->get_device()->CreateShaderResourceView(m_texture_resource.Get(), &srv_desc, m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    // Clean up
    stbi_image_free(data); // Free raw image data
}

moon::directx_texture2d::~directx_texture2d()
{
    m_texture_resource.Reset();
    m_upload_buffer.Reset();
    m_descriptor_heap.Reset();
    m_context = nullptr;
}

void moon::directx_texture2d::set_data(void* data, uint32_t size)
{
    MOON_PROFILE_FUNCTION();

    // input data validation
    uint32_t bpp = m_dxgi_format == DXGI_FORMAT_R8G8B8A8_UNORM ? 4 : 3;
    MOON_CORE_ASSERT(size == m_width * m_height * bpp, "Data must be entire texture!");

    command_list* command_list = m_context->get_command_list(m_context->get_current_buffer_index());

    // Get command list
    if (!command_list)
    {
        MOON_CORE_ERROR("Failed to get command list in set_data!");
        return;
    }

    // Transition to COPY_DEST state first (if it was in PIXEL_SHADER_RESOURCE state)
    command_list->transition_resource(m_texture_resource.Get(), ResourceState::FragmentShaderResource, ResourceState::CopyDest);

    // Create the upload buffer with proper size
    uint64_t required_buffer_size = GetRequiredIntermediateSize(m_texture_resource.Get(), 0, 1);

    // Upload heap properties
    D3D12_HEAP_PROPERTIES hp_upload = {};
    hp_upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    hp_upload.CreationNodeMask = 0;
    hp_upload.VisibleNodeMask = 0;

    // Upload buffer description
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Alignment = 0;
    rd.Width = required_buffer_size;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.SampleDesc.Count = 1;
    rd.SampleDesc.Quality = 0;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create upload buffer
    ComPtr<ID3D12Resource2> temp_upload_buffer;
    if (FAILED(m_context->get_device()->CreateCommittedResource(
        &hp_upload,
        D3D12_HEAP_FLAG_NONE,
        &rd,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&temp_upload_buffer)
    )))
    {
        MOON_CORE_ERROR("Failed to create upload buffer!");
        return;
    }

    // Prepare subresource data
    D3D12_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pData = data;
    subresource_data.RowPitch = m_width * bpp;
    subresource_data.SlicePitch = subresource_data.RowPitch * m_height;

    // Use UpdateSubresources to handle the upload
    UINT64 result = UpdateSubresources<1>(
        (ID3D12GraphicsCommandList*)command_list->get_native_handle(),
        m_texture_resource.Get(),
        temp_upload_buffer.Get(),
        0, 0, 1,
        &subresource_data
    );

    if (result == 0)
    {
        MOON_CORE_ERROR("UpdateSubresources failed!");
        return;
    }

    // Transition texture to shader resource state
    command_list->transition_resource(m_texture_resource.Get(), ResourceState::CopyDest, ResourceState::FragmentShaderResource);

    m_upload_buffer = temp_upload_buffer;
}

void moon::directx_texture2d::bind(uint32_t slot) const
{
    MOON_PROFILE_FUNCTION();

    ID3D12GraphicsCommandList* command_list = m_context->get_native_command_list();

    std::array<ID3D12DescriptorHeap*, 1> descriptor_heaps = { m_descriptor_heap.Get() };

    // Set the descriptor heap
    command_list->SetDescriptorHeaps((UINT)descriptor_heaps.size(), descriptor_heaps.data());

    // Get the GPU descriptor handle for the SRV
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();

    // Offset the handle by the specified slot (if there are multiple SRVs in the heap)
    gpu_handle.ptr += slot * m_context->get_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Bind the SRV to the shader at the specified slot
    command_list->SetGraphicsRootDescriptorTable(slot, gpu_handle);
}
