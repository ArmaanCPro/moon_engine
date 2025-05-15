#include "moonpch.h"
#include "d3d12_binding_set.h"

#include "d3d12_context.h"
#include "d3d12_texture.h"
#include "core/application.h"

namespace moon
{
    d3d12_binding_set::d3d12_binding_set(const binding_layout& layout, const ref<pipeline>& pipeline)
        :
        binding_set(layout), m_pipeline(pipeline)
    {
        create_descriptor_heaps();
    }

    void d3d12_binding_set::bind() const
    {
        MOON_PROFILE_FUNCTION();

        d3d12_context* context = (d3d12_context*)application::get().get_context();
        command_list* cmd_list = context->get_command_list(context->get_current_buffer_index());
        ID3D12GraphicsCommandList* native_cmd = (ID3D12GraphicsCommandList*)cmd_list->get_native_handle();

        if (m_descriptor_heap)
        {
            ID3D12DescriptorHeap* heaps[] = { m_descriptor_heap.Get() };
            native_cmd->SetDescriptorHeaps(_countof(heaps), heaps);
            native_cmd->SetGraphicsRootDescriptorTable(0, m_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
        }
    }

    void d3d12_binding_set::set_constant(uint32_t binding, const void* data, size_t size)
    {
        auto* context = (d3d12_context*)application::get().get_context();
        auto* device = context->get_native_device().Get();

        // create buffer if doesn't exist
        if (!m_constant_buffers.contains(binding))
        {
            CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(
                (size + 255) & ~255 // align to 256 bytes
            );

            ComPtr<ID3D12Resource2> upload_buffer;
            device->CreateCommittedResource(
                &heap_props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&upload_buffer)
            );

            // Create CBV
            D3D12_CPU_DESCRIPTOR_HANDLE handle = get_cpu_handle(binding);
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
            cbv_desc.BufferLocation = upload_buffer->GetGPUVirtualAddress();
            cbv_desc.SizeInBytes = (UINT)((size + 255) & ~255); // align to 256 bytes

            device->CreateConstantBufferView(&cbv_desc, handle);

            m_constant_buffers[binding] = upload_buffer;
        }

        // update buffer data
        void* mapped_data = nullptr;
        m_constant_buffers[binding]->Map(0, nullptr, &mapped_data);
        memcpy(mapped_data, data, size);
        m_constant_buffers[binding]->Unmap(0, nullptr);
    }

    void d3d12_binding_set::set_texture(uint32_t binding, const ref<texture2d>& texture)
    {
        MOON_PROFILE_FUNCTION();

        auto* context = (d3d12_context*)application::get().get_context();
        auto* device = context->get_native_device().Get();
        auto d3d_texture = std::dynamic_pointer_cast<d3d12_texture2d>(texture);

        // Create SRV
        D3D12_CPU_DESCRIPTOR_HANDLE handle = get_cpu_handle(binding);
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Format = d3d_texture->get_dxgi_format();
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;

        device->CreateShaderResourceView(
            d3d_texture->get_resource(),
            &srv_desc,
            handle
        );
    }

    void d3d12_binding_set::create_descriptor_heaps()
    {
        d3d12_context* context = (d3d12_context*)application::get().get_context();

        // create descriptor heaps
        uint32_t num_cbv_srv_uav = 0;
        for (const auto& element : m_layout.get_elements())
        {
            if (element.type != BindingResourceType::Sampler)
                num_cbv_srv_uav += element.array_size;
        }

        if (num_cbv_srv_uav > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = num_cbv_srv_uav;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;
            // TODO descriptor heap creation could be another facet of a device class abstraction
            if (FAILED(context->get_native_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptor_heap))))
            {
                MOON_CORE_ASSERT(false, "Failed to create descriptor heap!");
            }
        }

        m_descriptor_size = context->get_native_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE d3d12_binding_set::get_cpu_handle(uint32_t binding) const
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
            m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
            binding,
            m_descriptor_size
        );
        return handle;
    }
}
