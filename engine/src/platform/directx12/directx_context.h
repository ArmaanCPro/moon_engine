#pragma once

#include "moon/renderer/graphics_context.h"

#include "directx.h"
#include "directx_command_list.h"

#include <glm/vec4.hpp>

namespace moon
{
    class directx_context : public graphics_context
    {
    public:
        explicit directx_context(HWND window_handle);

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
        inline ComPtr<ID3D12Fence1>& get_fence() { return m_frames[m_current_buffer_index_].fence; }
        inline UINT& get_fence_value() { return m_frames[m_current_buffer_index_].fence_value; }
        inline uint32_t get_current_buffer_index() const { return m_current_buffer_index_; }

        // TEMP - Move into application
        static constexpr int s_frames_in_flight = 2;

        void signal_and_wait();

        ID3D12GraphicsCommandList* init_command_lists();

        ID3D12GraphicsCommandList* get_native_command_list() const;
        void execute_command_lists();

        void on_resize(uint32_t width, uint32_t height);

        command_list* get_command_list(uint32_t frame_index) override
        {
            return m_frames[frame_index].command_list.get();
        }

    private:
        void fetch_buffers();
        void release_buffers();

    private:
        struct frame_data
        {
            ComPtr<ID3D12CommandAllocator> allocator;
            scope<directx_command_list> command_list;
            ComPtr<ID3D12Fence1> fence;
            HANDLE fence_event;
            UINT fence_value;
        };
        std::array<frame_data, s_frames_in_flight> m_frames;

        HWND m_window_handle_;

        ComPtr<IDXGIFactory7> m_dxgi_factory_;

        ComPtr<ID3D12Device14> m_device_;
        ComPtr<ID3D12CommandQueue> m_command_queue_;

        ComPtr<IDXGISwapChain3> m_swap_chain_;
        ComPtr<ID3D12Resource> m_buffers[s_frames_in_flight];
        uint32_t m_current_buffer_index_ = 0; // index to back buffer

        ComPtr<ID3D12DescriptorHeap> m_rtv_heap_;
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtv_handles[s_frames_in_flight] {};

        bool vsync = true;

        glm::vec4 m_clear_color_ { 0.0f };

        // Debug Layer
#ifdef _DEBUG
        ComPtr<ID3D12Debug6> m_d3d12_debug;
        ComPtr<IDXGIDebug1> m_dxgi_debug;
#endif
    };
}
