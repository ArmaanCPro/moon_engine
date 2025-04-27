#include "moonpch.h"
#include "directx_context.h"

#include "directx.h"

#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    directx_context::directx_context(HWND window_handle)
        :
        m_window_handle_(window_handle)
    {
        MOON_CORE_ASSERT(window_handle, "Window handle is null!");
    }

    void directx_context::shutdown()
    {
        MOON_PROFILE_FUNCTION();

        // Debug layer
#ifdef _DEBUG
        if (m_dxgi_debug)
        {
            OutputDebugStringW(L"DXGI Repots living device objects:\n");
            m_dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY)); // DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL
        }

        m_dxgi_debug.Reset();
        m_d3d12_debug.Reset();
#endif

        m_swap_chain_.Reset();

        m_command_list_.Reset();
        m_command_allocator_.Reset();

        if (m_fence_event_)
            CloseHandle(m_fence_event_);

        m_fence_.Reset();
        m_command_queue_.Reset();
        m_device_.Reset();
        m_dxgi_factory_.Reset();
    }

    void directx_context::begin_frame()
    {
        MOON_PROFILE_FUNCTION();

        // Get the current back buffer index
        m_current_buffer_index_ = m_swap_chain_->GetCurrentBackBufferIndex();

        // Transition the back buffer from PRESENT to RENDER_TARGET
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_buffers[m_current_buffer_index_].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_command_list_->ResourceBarrier(1, &barrier);

        // Set the render target for this frame but do not clear it here
        m_command_list_->OMSetRenderTargets(1, &m_rtv_handles[m_current_buffer_index_], FALSE, nullptr);
    }

    void directx_context::end_frame()
    {
        MOON_PROFILE_FUNCTION();

        D3D12_RESOURCE_BARRIER barr;
        barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;    // consider floating barriers
        barr.Transition.pResource = m_buffers[m_current_buffer_index_].Get();
        barr.Transition.Subresource = 0;
        barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        m_command_list_->ResourceBarrier(1, &barr);
    }

    void directx_context::set_clear_color(const glm::vec4& color)
    {
        m_clear_color_ = color;
    }

    void directx_context::clear()
    {
        MOON_PROFILE_FUNCTION();

        const float clear_color[] = { m_clear_color_.r, m_clear_color_.g, m_clear_color_.b, m_clear_color_.a };
        m_command_list_->ClearRenderTargetView(m_rtv_handles[m_current_buffer_index_], clear_color, 0, nullptr);
    }

    void directx_context::swap_buffers()
    {
        MOON_PROFILE_FUNCTION();

        UINT sync_interval = vsync ? 1 : 0;
        UINT present_flags = vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING;

        m_swap_chain_->Present(sync_interval, present_flags);
    }

    void directx_context::signal_and_wait()
    {
        MOON_PROFILE_FUNCTION();

        m_command_queue_->Signal(m_fence_.Get(), ++m_fence_value_);
        if (SUCCEEDED(m_fence_->SetEventOnCompletion(m_fence_value_, m_fence_event_)))
        {
            if (SUCCEEDED(m_fence_->SetEventOnCompletion(m_fence_value_, m_fence_event_)))
            {
                if (WaitForSingleObject(m_fence_event_, 20000) != WAIT_OBJECT_0)
                {
                    MOON_CORE_ERROR("Failed to wait for fence event!");
                }
            }
            else
            {
                MOON_CORE_ERROR("Failed to set fence event!");
            }
        }
    }

    ID3D12GraphicsCommandList10* directx_context::init_command_list()
    {
        MOON_PROFILE_FUNCTION();

        m_command_allocator_->Reset();
        m_command_list_->Reset(m_command_allocator_.Get(), nullptr);
        return m_command_list_.Get();
    }

    void directx_context::execute_command_list()
    {
        MOON_PROFILE_FUNCTION();

        if (SUCCEEDED(m_command_list_->Close()))
        {
            ID3D12CommandList* lists[] = { m_command_list_.Get() };
            m_command_queue_->ExecuteCommandLists(1, lists);
            signal_and_wait();
        }
    }

    void directx_context::on_resize(uint32_t width, uint32_t height)
    {
        MOON_PROFILE_FUNCTION();

        // not sure if flushing is necessary
        flush(s_frames_in_flight);
        release_buffers();

        if (FAILED(m_swap_chain_->ResizeBuffers(s_frames_in_flight, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
        {
            MOON_CORE_ERROR("Failed to resize swap chain buffers.");
            return;
        }

        fetch_buffers();
    }

    void directx_context::fetch_buffers()
    {
        for (size_t i = 0; i < s_frames_in_flight; ++i)
        {
            if (FAILED(m_swap_chain_->GetBuffer((UINT)i, IID_PPV_ARGS(&m_buffers[i]))))
            {
                MOON_CORE_ERROR("Failed to get buffer!");
            }

            D3D12_RENDER_TARGET_VIEW_DESC rtvd = {};
            rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvd.Texture2D.MipSlice = 0;
            rtvd.Texture2D.PlaneSlice = 0;
            m_device_->CreateRenderTargetView(m_buffers[i].Get(), &rtvd, m_rtv_handles[i]);
        }
    }

    void directx_context::release_buffers()
    {
        for (size_t i = 0; i < s_frames_in_flight; ++i)
        {
            m_buffers[i].Reset();
        }
    }

    void directx_context::init()
    {
        MOON_PROFILE_FUNCTION();

        // TODO: Seperate DebugLayer into a seperate singleton class
        // Debug layer
#ifdef _DEBUG
#define D3DCOMPILE_DEBUG 1
        {
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12_debug))))
            {
                m_d3d12_debug->EnableDebugLayer();

                if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgi_debug))))
                {
                    m_dxgi_debug->EnableLeakTrackingForThread();
                }
            }
        }
#endif

        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgi_factory_))))
        {
            MOON_CORE_ERROR("Failed to create DXGI factory!");
        }
        
        if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device_))))
        {
            MOON_CORE_ERROR("Failed to create D3D12 device!");
        }

        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        cmd_queue_desc.NodeMask = 0;
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        if (FAILED(m_device_->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&m_command_queue_))))
        {
            MOON_CORE_ERROR("Failed to create command queue!");
        }

        if (FAILED(m_device_->CreateFence(m_fence_value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence_))))
        {
            MOON_CORE_ERROR("Failed to create fence!");
        }

        m_fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fence_event_)
        {
            MOON_CORE_ERROR("Failed to create fence event!");
        }

        if (FAILED(m_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator_))))
        {
            MOON_CORE_ERROR("Failed to create command allocator!");
        }

        if (FAILED(m_device_->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_command_list_))))
        {
            MOON_CORE_ERROR("Failed to create command list!");
        }

        // === Swapchain ===
        // describe swap chain
        DXGI_SWAP_CHAIN_DESC1 swd = {};
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfd = {};

        RECT rect;
        GetWindowRect(m_window_handle_, &rect); // TODO: make sure this is right and not GetClientRect
        swd.Width = rect.right - rect.left;
        swd.Height = rect.bottom - rect.top;
        swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swd.Stereo = false;
        swd.SampleDesc.Count = 1;
        swd.SampleDesc.Quality = 0;
        swd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swd.BufferCount = 2;
        swd.Scaling = DXGI_SCALING_STRETCH;
        swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        sfd.Windowed = true;

        // create swap chain
        ComPtr<IDXGISwapChain1> sc1;
        m_dxgi_factory_->CreateSwapChainForHwnd(m_command_queue_.Get(), m_window_handle_, &swd, &sfd, nullptr, &sc1);
        if (!sc1 || FAILED(sc1->QueryInterface(IID_PPV_ARGS(&m_swap_chain_))))
        {
            MOON_CORE_ERROR("Failed to create swap chain!");
        }

        // create RTV heap
        D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
        desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc_heap_desc.NumDescriptors = s_frames_in_flight;
        desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc_heap_desc.NodeMask = 0;
        if (FAILED(m_device_->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&m_rtv_heap_))))
        {
            MOON_CORE_ERROR("Failed to create RTV heap!");
        }

        auto first_handle = m_rtv_heap_->GetCPUDescriptorHandleForHeapStart();
        auto rtv_handle_size = m_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (size_t i = 0; i < s_frames_in_flight; ++i)
        {
            m_rtv_handles[i] = first_handle;
            first_handle.ptr += rtv_handle_size;
        }

        fetch_buffers();
    }
}
