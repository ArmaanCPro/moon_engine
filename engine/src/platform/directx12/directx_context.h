#pragma once

#include "moon/renderer/graphics_context.h"

#include "directx.h"

#include <glm/vec4.hpp>

namespace moon
{
    class directx_context : public graphics_context
    {
    public:
        directx_context(HWND window_handle);

        void init() override;
        void shutdown();

        void begin_frame() override;
        void end_frame() override;
        void set_clear_color(const glm::vec4& color);
        void clear() const;
        void swap_buffers() override;

        void set_vsync(bool value) { vsync = value; }

        void flush(size_t count) override { for (size_t i = 0; i < count; i++) signal_and_wait(); }

        inline ComPtr<IDXGIFactory7>& get_dxgi_factory() { return m_dxgi_factory_; }
        inline ComPtr<ID3D12Device14>& get_device() { return m_device_; }
        inline ComPtr<ID3D12CommandQueue>& get_command_queue() { return m_command_queue_; }
        inline ComPtr<ID3D12DescriptorHeap>& get_rtv_heap() { return m_rtv_heap_; }

        // TEMP
        static constexpr int s_frames_in_flight = 2;

        void signal_and_wait();
        ID3D12GraphicsCommandList10* init_command_lists();
        ID3D12GraphicsCommandList10* get_command_list() const;
        void execute_command_lists();

        ID3D12GraphicsCommandList10* begin_resource_upload();
        void end_resource_upload();

        void on_resize(uint32_t width, uint32_t height);

    private:
        void fetch_buffers();
        void release_buffers();

    private:
        HWND m_window_handle_;

        ComPtr<IDXGIFactory7> m_dxgi_factory_;

        ComPtr<ID3D12Device14> m_device_;
        ComPtr<ID3D12CommandQueue> m_command_queue_;

        std::array<ComPtr<ID3D12CommandAllocator>, s_frames_in_flight> m_command_allocators;
        std::array<ComPtr<ID3D12GraphicsCommandList10>, s_frames_in_flight> m_command_lists;

        ComPtr<ID3D12CommandAllocator> m_temp_allocator;
        ComPtr<ID3D12GraphicsCommandList10> m_temp_list;

        ComPtr<ID3D12Fence1> m_fence_;
        UINT64 m_fence_value_ = 0;
        HANDLE m_fence_event_ = nullptr;

        ComPtr<IDXGISwapChain3> m_swap_chain_;
        ComPtr<ID3D12Resource> m_buffers[s_frames_in_flight];
        size_t m_current_buffer_index_ = 0; // index to back buffer

        ComPtr<ID3D12DescriptorHeap> m_rtv_heap_;
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtv_handles[s_frames_in_flight] {};

        bool vsync = true;

        glm::vec4 m_clear_color_ { 0.0f };

        // Debug Layer
#ifdef _DEBUG
        ComPtr<ID3D12Debug6> m_d3d12_debug;
        ComPtr<IDXGIDebug1> m_dxgi_debug;
#endif

        friend class windows_window;
    };
}
