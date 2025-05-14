#include "moonpch.h"
#include "d3d12_framebuffer.h"

#include "d3d12_context.h"
#include "core/application.h"

namespace moon
{
    d3d12_framebuffer::d3d12_framebuffer(const framebuffer_spec& spec)
        :
        m_spec(spec),
        m_color_current_state(D3D12_RESOURCE_STATE_COMMON),
        m_depth_current_state(D3D12_RESOURCE_STATE_COMMON)
    {
        invalidate();
    }

    d3d12_framebuffer::~d3d12_framebuffer()
    {
        cleanup();
    }

    void d3d12_framebuffer::invalidate()
    {
        if (m_color_texture)
            cleanup();

        create_textures();
    }

    void d3d12_framebuffer::bind()
    {
        auto* context = (d3d12_context*)application::get().get_context();
        // command_list* cmd = context->get_command_list(context->get_current_buffer_index());
        ID3D12GraphicsCommandList* native_cmd = context->get_native_command_list();

        if (m_color_current_state == D3D12_RESOURCE_STATE_COMMON)
        {
            // Transition color texture to render target
            CD3DX12_RESOURCE_BARRIER color_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_color_texture.Get(), m_color_current_state, D3D12_RESOURCE_STATE_RENDER_TARGET);
            // CD3DX12_RESOURCE_BARRIER depth_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_depth_texture.Get(), m_depth_current_state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

            native_cmd->ResourceBarrier(1, &color_barrier);
            m_color_current_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        }

        // Set render targets
        native_cmd->OMSetRenderTargets(1, &m_color_rtv_handle, TRUE, &m_depth_dsv_handle);

        // Set viewport and scissor rect
        CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)m_spec.width, (float)m_spec.height);

        CD3DX12_RECT scissor_rect = CD3DX12_RECT(0, 0, (LONG)m_spec.width, (LONG)m_spec.height);

        native_cmd->RSSetViewports(1, &viewport);
        native_cmd->RSSetScissorRects(1, &scissor_rect);
    }

    void d3d12_framebuffer::unbind()
    {
        if (m_color_current_state == D3D12_RESOURCE_STATE_RENDER_TARGET)
        {
            auto* context = (d3d12_context*)application::get().get_context();
            // command_list* cmd = context->get_command_list(context->get_current_buffer_index());
            ID3D12GraphicsCommandList* native_cmd = context->get_native_command_list();

            // Transition color texture back to common
            CD3DX12_RESOURCE_BARRIER color_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_color_texture.Get(), m_color_current_state, D3D12_RESOURCE_STATE_COMMON);
            // CD3DX12_RESOURCE_BARRIER depth_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_depth_texture.Get(), m_depth_current_state, D3D12_RESOURCE_STATE_COMMON);

            native_cmd->ResourceBarrier(1, &color_barrier);
            m_color_current_state = D3D12_RESOURCE_STATE_COMMON;
        }
    }

    void d3d12_framebuffer::resize(uint32_t width, uint32_t height)
    {
        m_spec.width = width;
        m_spec.height = height;
        invalidate();
    }

    void d3d12_framebuffer::create_textures()
    {
        auto* context = (d3d12_context*)application::get().get_context();
        ID3D12Device* device = context->get_native_device().Get();

        // Create color texture
        CD3DX12_RESOURCE_DESC color_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, m_spec.width, m_spec.height);
        color_desc.SampleDesc.Count = m_spec.samples;
        color_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_DEFAULT);

        device->CreateCommittedResource(
            &heap_props,
            D3D12_HEAP_FLAG_NONE,
            &color_desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&m_color_texture)
        );

        // Create depth texture
        CD3DX12_RESOURCE_DESC depth_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_spec.width, m_spec.height);
        depth_desc.SampleDesc.Count = m_spec.samples;
        depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // TODO: Make sure this is the correct format
        depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depth_clear = {};
        depth_clear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_clear.DepthStencil.Depth = 1.0f;
        depth_clear.DepthStencil.Stencil = 0;

        device->CreateCommittedResource(
            &heap_props,
            D3D12_HEAP_FLAG_NONE,
            &depth_desc,
            D3D12_RESOURCE_STATE_COMMON,
            &depth_clear,
            IID_PPV_ARGS(&m_depth_texture)
        );

        // Create RTV and DSV heaps
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.NumDescriptors = 1;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtv_heap_desc.NodeMask = 0;
        HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtv_heap));
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create RTV heap");

        m_color_rtv_handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();

        D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
        dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsv_heap_desc.NumDescriptors = 1;
        dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsv_heap_desc.NodeMask = 0;
        hr = device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&m_dsv_heap));
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create DSV heap");

        m_depth_dsv_handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();

        // Create RTV and DSV descriptors
        D3D12_RENDER_TARGET_VIEW_DESC color_rtv_desc = {};
        color_rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        color_rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        color_rtv_desc.Texture2D.MipSlice = 0;
        color_rtv_desc.Texture2D.PlaneSlice = 0;
        device->CreateRenderTargetView(m_color_texture.Get(), &color_rtv_desc, m_color_rtv_handle);

        D3D12_DEPTH_STENCIL_VIEW_DESC depth_dsv_desc = {};
        depth_dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depth_dsv_desc.Texture2D.MipSlice = 0;
        device->CreateDepthStencilView(m_depth_texture.Get(), &depth_dsv_desc, m_depth_dsv_handle);
    }

    void d3d12_framebuffer::cleanup()
    {
        auto* context = (d3d12_context*)application::get().get_context();
        context->flush(d3d12_context::s_frames_in_flight);

        // Release RTV and DSV heaps
        m_rtv_heap.Reset();
        m_dsv_heap.Reset();
        m_color_texture.Reset();
        m_depth_texture.Reset();
        m_color_current_state = D3D12_RESOURCE_STATE_COMMON;
        m_depth_current_state = D3D12_RESOURCE_STATE_COMMON;
    }
}
