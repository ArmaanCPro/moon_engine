#include "moonpch.h"
#include "directx_shader.h"

#include "directx_context.h"
#include "core/application.h"

#include "directx.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>

namespace moon
{
    directx_shader::directx_shader(ShaderType type, std::string_view filepath)
    {
        MOON_PROFILE_FUNCTION();

        m_type = type;

        m_data = read_file(filepath);
        MOON_CORE_ASSERT(!m_data.empty(), "Failed to read shader file!");

        // Extract the name from the filepath
        auto last_slash = filepath.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = filepath.rfind('.');

        auto count = last_dot == std::string::npos ? filepath.size() - last_slash : last_dot - last_slash;
        m_name = filepath.substr(last_slash, count);

        // ROOT SIG (we only need to do this to extract an embedded root sig, otherwise m_data is the rootsig)
        if (m_type != ShaderType::RootSignature)
        {
            ComPtr<ID3DBlob> blob = create_blob();
            ComPtr<ID3DBlob> rootsig_blob;

            HRESULT hr = D3DGetBlobPart(
                blob->GetBufferPointer(),
                blob->GetBufferSize(),
                D3D_BLOB_ROOT_SIGNATURE,
                0,
                &rootsig_blob
            );
            MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to get root signature blob!");

            auto* context = (directx_context*)(application::get().get_context());
            auto* device = context->get_device().Get();

            device->CreateRootSignature(0, rootsig_blob->GetBufferPointer(), rootsig_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
        }
    }

    directx_shader::directx_shader(std::string_view vertex_path, std::string_view fragment_path)
    {
        MOON_PROFILE_FUNCTION();

        m_type = ShaderType::VertexAndFragment;

        m_data = read_file(vertex_path) + read_file(fragment_path);
        MOON_CORE_ASSERT(!m_data.empty(), "Failed to read shader file!");

        // Extract the name from the filepath
        auto last_slash = vertex_path.find_last_of("/\\");
        last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
        auto last_dot = vertex_path.rfind('.');

        auto count = last_dot == std::string::npos ? vertex_path.size() - last_slash : last_dot - last_slash;
        m_name = vertex_path.substr(last_slash, count);

        // ROOT SIG (we only need to do this to extract an embedded root sig, otherwise m_data is the rootsig)
        if (m_type != ShaderType::RootSignature)
        {
            ComPtr<ID3DBlob> blob = create_blob();
            ComPtr<ID3DBlob> rootsig_blob;
            ComPtr<ID3DBlob> error_blob;

            HRESULT hr = D3DGetBlobPart(
                blob->GetBufferPointer(),
                blob->GetBufferSize(),
                D3D_BLOB_ROOT_SIGNATURE,
                0,
                &rootsig_blob
            );

            if (FAILED(hr))
            {
                // If extracting failed, create a default root signature instead
                MOON_CORE_WARN("Failed to extract root signature from shader, creating default one instead");

                // Create a default root signature programmatically
                CD3DX12_ROOT_PARAMETER1 rootParameters[1];
                CD3DX12_DESCRIPTOR_RANGE1 ranges[2];

                // CBV for matrix
                ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
                // SRV for textures
                ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

                rootParameters[0].InitAsDescriptorTable(2, ranges);

                D3D12_STATIC_SAMPLER_DESC sampler = {};
                sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                sampler.MipLODBias = 0;
                sampler.MaxAnisotropy = 0;
                sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
                sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                sampler.MinLOD = 0.0f;
                sampler.MaxLOD = D3D12_FLOAT32_MAX;
                sampler.ShaderRegister = 0;
                sampler.RegisterSpace = 0;
                sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
                rootSignatureDesc.Init_1_1(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &rootsig_blob, &error_blob);

                if (FAILED(hr))
                {
                    if (error_blob)
                    {
                        MOON_CORE_ERROR("Root signature serialization error: {0}", (char*)error_blob->GetBufferPointer());
                    }
                    MOON_CORE_ASSERT(false, "Failed to serialize root signature!");
                }
            }

            auto* context = (directx_context*)(application::get().get_context());
            auto* device = context->get_device().Get();

            hr = device->CreateRootSignature(0, rootsig_blob->GetBufferPointer(), rootsig_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
            MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create root signature!");
        }
    }

    directx_shader::directx_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src)
    {
        MOON_PROFILE_FUNCTION();

        // TODO: support this in future
        MOON_CORE_ASSERT(false, "DirectX does not support vertex and fragment shaders in one file!");
        // m_name = name;
        //m_data = vertex_src + fragment_src;
    }

    directx_shader::~directx_shader()
    {
        m_constant_buffers.clear();
        m_cbv_descriptor_heap.Reset();
        m_srv_descriptor_heap.Reset();
    }

    ComPtr<ID3DBlob> directx_shader::create_blob() const
    {
        MOON_PROFILE_FUNCTION();

        ComPtr<ID3DBlob> blob;
        HRESULT hr = D3DCreateBlob(m_data.size(), &blob);
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create blob!");
        memcpy(blob->GetBufferPointer(), m_data.data(), m_data.size());
        return blob;
    }

    void directx_shader::bind() const
    {
        MOON_PROFILE_FUNCTION();

        auto* context = (directx_context*)application::get().get_context();
        ID3D12GraphicsCommandList* cmd_list = context->get_native_command_list();

        auto* native_cmd = (ID3D12GraphicsCommandList*)context->get_command_list(context->get_current_buffer_index())->get_native_handle();
        native_cmd->SetGraphicsRootSignature(m_root_signature.Get());

        if (m_cbv_descriptor_heap)
        {
            ID3D12DescriptorHeap* heaps[] = { m_cbv_descriptor_heap.Get() };
            cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);

            // Bind any persistent constant buffers if needed
            for ([[maybe_unused]] const auto& buffer : m_constant_buffers)
            {
                // UINT descriptorSize = context->get_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                cmd_list->SetGraphicsRootDescriptorTable(0, m_cbv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
            }
        }

        // Set primitive topology (usually triangles for most 2D/3D rendering)
        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void directx_shader::unbind() const
    {

    }

    void directx_shader::set_int(std::string_view name, int value)
    {}

    void directx_shader::set_int_array(std::string_view name, int* values, uint32_t count)
    {
        MOON_PROFILE_FUNCTION();

        MOON_CORE_ASSERT(count <= 32, "DirectX does not support int arrays!");

        auto* context = (directx_context*)application::get().get_context();

        ID3D12GraphicsCommandList* cmd_list = context->get_native_command_list();

        if (!m_srv_descriptor_heap)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = count;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            HRESULT hr = context->get_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srv_descriptor_heap));
            MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create descriptor heap!");
        }

        ID3D12DescriptorHeap* heaps[] =  { m_srv_descriptor_heap.Get() };
        cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);

        // UINT descriptor_size = context->get_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart();

        // TODO: Consider shader reflection or dynamic root signature creation
        constexpr UINT root_parameter_index = 1;
        cmd_list->SetGraphicsRootDescriptorTable(root_parameter_index, gpu_handle);
    }

    void directx_shader::set_float(std::string_view name, float value)
    {

    }

    void directx_shader::set_float2(std::string_view name, const glm::vec2& value)
    {}

    void directx_shader::set_float3(std::string_view name, const glm::vec3& value)
    {}

    void directx_shader::set_float4(std::string_view name, const glm::vec4& value)
    {}

    void directx_shader::set_mat4(std::string_view name, const glm::mat4& value)
    {
        MOON_PROFILE_FUNCTION();

        auto* context = (directx_context*)application::get().get_context();
        ID3D12GraphicsCommandList* cmd_list = context->get_native_command_list();

        glm::mat4 transposed = glm::transpose(value);

        const UINT constant_buffer_size = (sizeof(glm::mat4) + 255) & ~255;

        ComPtr<ID3D12Resource2> constant_buffer;
        CD3DX12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(constant_buffer_size);

        HRESULT hr = context->get_device()->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &buffer_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&constant_buffer)
        );

        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create constant buffer!");

        void* mapped_data = nullptr;
        CD3DX12_RANGE read_range(0, 0);

        hr = constant_buffer->Map(0, &read_range, &mapped_data);
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to map constant buffer!");

        memcpy(mapped_data, &transposed, sizeof(glm::mat4));
        constant_buffer->Unmap(0, nullptr);

        if (!m_cbv_descriptor_heap)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 33; // CBV & SRV
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            hr = context->get_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cbv_descriptor_heap));
            MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create descriptor heap!");
        }

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
        cbv_desc.BufferLocation = constant_buffer->GetGPUVirtualAddress();
        cbv_desc.SizeInBytes = constant_buffer_size;

        context->get_device()->CreateConstantBufferView(&cbv_desc, m_cbv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        ID3D12DescriptorHeap* heaps[] =  { m_cbv_descriptor_heap.Get() };
        cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);

        cmd_list->SetGraphicsRootDescriptorTable(0, m_cbv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());

        m_constant_buffers.push_back(constant_buffer);
    }

    std::string directx_shader::read_file(std::string_view filepath)
    {
        MOON_PROFILE_FUNCTION();


        std::string result;

        std::ifstream in(filepath.data(), std::ios::binary);

        if (in)
        {
            in.seekg(0, std::ios::end);
            std::streampos size = in.tellg();
            if (size != -1)
            {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], result.size());
                in.close();
            }
            else
            {
                MOON_CORE_ERROR("Failed to read file: {0}", filepath);
            }
        }
        else
        {
            MOON_CORE_ERROR("Failed to open file: {0}", filepath);
        }

        return result;
    }
}
