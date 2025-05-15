#include "moonpch.h"
#include "d3d12_root_signature.h"

#include "d3d12_context.h"
#include "core/application.h"

#include <ranges>

namespace moon
{
    void d3d12_root_signature_desc::build_from_layout(const binding_layout& layout)
    {
        // Store ranges and parameters as member variables to maintain their lifetime
        m_ranges.clear();
        m_parameters.clear();

        // Group bindings by set
        std::map<uint32_t, std::vector<size_t>> ranges_by_set;

        // create all ranges
        for (const auto& element : layout)
        {
            D3D12_DESCRIPTOR_RANGE range = {};

            switch (element.type)
            {
            case BindingResourceType::UniformBuffer:
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                break;
            case BindingResourceType::StorageBuffer:
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                break;
            case BindingResourceType::Texture:
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                break;
            case BindingResourceType::Sampler:
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                break;
            default:
                MOON_CORE_ASSERT(false, "Unknown binding type");
                continue;
            }

            range.NumDescriptors = element.array_size;
            range.BaseShaderRegister = element.binding;
            range.RegisterSpace = element.set;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            size_t range_index = m_ranges.size();
            m_ranges.push_back(range);
            ranges_by_set[element.set].push_back(range_index);
        }

        // create a descriptor table for each set
        for (const auto& [set, range_indices] : ranges_by_set)
        {
            D3D12_ROOT_PARAMETER parameter = {};
            parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            parameter.DescriptorTable.NumDescriptorRanges = (UINT)range_indices.size();

            // store the ranges for this parameter
            std::vector<D3D12_DESCRIPTOR_RANGE> set_ranges;
            for (size_t idx : range_indices)
            {
                set_ranges.push_back(m_ranges[idx]);
            }
            m_parameter_ranges.push_back(std::move(set_ranges));

            // point to the stored ranges
            parameter.DescriptorTable.pDescriptorRanges = m_parameter_ranges.back().data();
            parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            m_parameters.push_back(parameter);
        }

        // setup the root signature description
        m_static_sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

        m_native_desc = {};
        m_native_desc.NumParameters = (UINT)m_parameters.size();
        m_native_desc.pParameters = m_parameters.data();
        m_native_desc.NumStaticSamplers = 1;
        m_native_desc.pStaticSamplers = &m_static_sampler;
        m_native_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    }

    void d3d12_root_signature::init(const d3d12_root_signature_desc& desc)
    {
        ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> error;

        HRESULT hr = D3D12SerializeRootSignature(
            &desc.get_desc(),
            D3D_ROOT_SIGNATURE_VERSION_1_0,
            &serialized,
            &error
        );

        if (error)
        {
            MOON_CORE_ERROR("{0}", (const char*)error->GetBufferPointer());
        }
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to serialize root signature");

        auto* context = (d3d12_context*)application::get().get_context();
        hr = context->get_native_device()->CreateRootSignature(
            0,
            serialized->GetBufferPointer(),
            serialized->GetBufferSize(),
            IID_PPV_ARGS(&m_root_signature)
        );

        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create root signature");
    }
}
