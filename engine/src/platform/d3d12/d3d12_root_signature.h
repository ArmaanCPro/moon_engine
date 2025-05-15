#pragma once

#include "moon/renderer/root_signature.h"

#include "d3d12_include.h"

namespace moon
{
    class d3d12_root_signature_desc : public root_signature_desc
    {
    public:
        explicit d3d12_root_signature_desc(const binding_layout& layout)
        {
            build_from_layout(layout);
        }

        // TEMP
        const D3D12_ROOT_SIGNATURE_DESC& get_desc() const { return m_native_desc; }

        const void* get_native_desc() const override
        {
            return (void*)&m_native_desc;
        }

    private:
        void build_from_layout(const binding_layout& layout);

    private:
        std::vector<D3D12_DESCRIPTOR_RANGE> m_ranges;
        std::vector<D3D12_ROOT_PARAMETER> m_parameters;
        std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_parameter_ranges;
        D3D12_STATIC_SAMPLER_DESC m_static_sampler;
        D3D12_ROOT_SIGNATURE_DESC m_native_desc;
    };

    class d3d12_root_signature : public root_signature
    {
    public:
        explicit d3d12_root_signature(const root_signature_desc& desc)
        {
            init((const d3d12_root_signature_desc&)desc);
        }

        // TEMP
        ID3D12RootSignature* get() const { return m_root_signature.Get(); }

        void* get_native_handle() const override { return (void*)m_root_signature.Get(); }

    private:
        void init(const d3d12_root_signature_desc& desc);
    private:
        ComPtr<ID3D12RootSignature> m_root_signature;
    };
}
