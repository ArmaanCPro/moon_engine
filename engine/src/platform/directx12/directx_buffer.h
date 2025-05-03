#pragma once

#include "directx.h"
#include "moon/renderer/buffer.h"

namespace moon
{
    class directx_vertex_buffer : public vertex_buffer
    {
    public:
        directx_vertex_buffer(uint32_t size);
        directx_vertex_buffer(const float* vertices, uint32_t size);
        ~directx_vertex_buffer() override;

        void bind() const override;
        void unbind() const override {}

        void set_data(const void* data, uint32_t size) override;

        const buffer_layout& get_layout() const override { return m_layout; }
        void set_layout(const buffer_layout& layout) override { m_layout = layout; }

    private:
        ComPtr<ID3D12Resource> m_buffer;
        D3D12_VERTEX_BUFFER_VIEW m_view;
        buffer_layout m_layout;

        ID3D12Device14* m_device;
        ID3D12GraphicsCommandList10* m_command_list;
    };

    class directx_index_buffer : public index_buffer
    {
    public:
        directx_index_buffer(const uint32_t* indices, uint32_t count);
        ~directx_index_buffer() override;

        inline uint32_t get_count() const override { return m_count; }

        void bind() const override;
        void unbind() const override {}

    private:
        ComPtr<ID3D12Resource> m_buffer;
        ID3D12GraphicsCommandList10* m_command_list;
        D3D12_INDEX_BUFFER_VIEW m_view;
        uint32_t m_count;
    };
}
