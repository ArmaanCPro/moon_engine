#pragma once

#include "moon/renderer/texture.h"
#include "d3d12_include.h"
#include "d3d12_context.h"

namespace moon
{
    class d3d12_texture2d : public texture2d
    {
    public:
        d3d12_texture2d(uint32_t width, uint32_t height);
        explicit d3d12_texture2d(std::string_view path);
        ~d3d12_texture2d() override = default;

        uint32_t get_width() const override { return m_width; }
        uint32_t get_height() const override { return m_height; }
        uint32_t get_renderer_id() const override { return 0; } // no ids in dx12

        void set_data(void* data, uint32_t size) override;

        void bind(uint32_t slot) const override;

        bool operator==(const texture& other) const override
        {
            return m_texture_resource == ((d3d12_texture2d&)other).m_texture_resource;
        }

        DXGI_FORMAT get_dxgi_format() const { return m_dxgi_format; }
        ID3D12Resource* get_resource() const { return m_texture_resource.Get(); }

    private:
        std::string m_path;
        uint32_t m_width, m_height;

        // temp
        d3d12_context* m_context;

        ComPtr<ID3D12Resource2> m_texture_resource;
        ComPtr<ID3D12Resource2> m_upload_buffer;
        ComPtr<ID3D12DescriptorHeap> m_descriptor_heap;
        DXGI_FORMAT m_dxgi_format;
    };
}
