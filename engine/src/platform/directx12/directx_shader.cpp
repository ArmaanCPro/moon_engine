#include "moonpch.h"
#include "directx_shader.h"

#include "directx_context.h"
#include "core/application.h"

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
        m_root_signature.Reset();
    }

    void directx_shader::bind() const
    {
        MOON_PROFILE_FUNCTION();

        auto* context = (directx_context*)application::get().get_context();
        ID3D12GraphicsCommandList* cmd_list = context->get_command_list();

        cmd_list->SetGraphicsRootSignature(m_root_signature.Get());

        if (m_cbv_descriptor_heap)
        {
            ID3D12DescriptorHeap* heaps[] = { m_cbv_descriptor_heap.Get() };
            cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);

            // Bind any persistent constant buffers if needed
            for (const auto& buffer : m_constant_buffers)
            {
                cmd_list->SetGraphicsRootConstantBufferView(0, buffer->GetGPUVirtualAddress());
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

        ID3D12GraphicsCommandList* cmd_list = context->get_command_list();

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
        ID3D12GraphicsCommandList* cmd_list = context->get_command_list();

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
            desc.NumDescriptors = 1;
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

        UINT root_parameter_index = 0;

        cmd_list->SetGraphicsRootDescriptorTable(root_parameter_index, m_cbv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());

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
