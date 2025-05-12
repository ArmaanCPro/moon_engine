#pragma once

#include "moon/renderer/framebuffer.h"
#include "d3d12_include.h"

namespace moon
{
    class d3d12_framebuffer : public framebuffer
    {
    public:
        explicit d3d12_framebuffer(const framebuffer_spec& spec);
        ~d3d12_framebuffer() override;

        void invalidate();
        void bind() override;
        void unbind() override;
        void resize(uint32_t width, uint32_t height) override;

        uint32_t get_color_attachment_renderer_id() const override { return 0; } // doesn't exist in directx
        const framebuffer_spec& get_spec() const override { return m_spec; }

    private:
        void create_textures();
        void cleanup();

    private:
        framebuffer_spec m_spec;

        ComPtr<ID3D12Resource> m_color_texture;
        ComPtr<ID3D12Resource> m_depth_texture;

        ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
        ComPtr<ID3D12DescriptorHeap> m_dsv_heap;

        CD3DX12_CPU_DESCRIPTOR_HANDLE m_color_rtv_handle;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_depth_dsv_handle;

        D3D12_RESOURCE_STATES m_color_current_state;
        D3D12_RESOURCE_STATES m_depth_current_state;
    };
}
