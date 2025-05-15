#pragma once

#include "d3d12_shader.h"
#include "moon/renderer/binding_set.h"

namespace moon
{
    class d3d12_binding_set : public binding_set
    {
    public:
        d3d12_binding_set(const binding_layout& layout, const ref<pipeline>& pipeline);

        void bind() const override;
        void unbind() const override {}

        void set_constant(uint32_t binding, const void* data, size_t size) override;
        void set_texture(uint32_t binding, const ref<texture2d>& texture) override;
        void set_uniform_buffer(uint32_t binding, const ref<vertex_buffer>& buffer) override {}

    private:
        void create_descriptor_heaps();
        D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(uint32_t binding) const;
    private:
        ref<pipeline> m_pipeline;
        // cbv, srv, uav heap
        ComPtr<ID3D12DescriptorHeap> m_descriptor_heap;
        UINT m_descriptor_size = 0;
        std::unordered_map<uint32_t, ComPtr<ID3D12Resource2>> m_constant_buffers;
    };
}
