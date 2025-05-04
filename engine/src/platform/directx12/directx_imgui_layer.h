#pragma once

#include "moon/imgui/imgui_layer.h"
#include "directx.h"
#include "events/application_event.h"

namespace moon
{
    class directx_imgui_layer : public imgui_layer
    {
    public:
        directx_imgui_layer();
        ~directx_imgui_layer() override;

        void on_attach() override;
        void on_detach() override;
        void begin() override;
        void end() override;
        void on_imgui_render() override;
        void on_event(event& e) override;

    private:
        bool m_initialized = false;
        ComPtr<ID3D12DescriptorHeap> m_srv_heap;

        // Descriptor tracking helper
        struct DescriptorAllocator
        {
            ID3D12DescriptorHeap* heap = nullptr;
            UINT descriptor_size = 0;
            UINT capacity = 0;
            UINT current_offset = 0;

            void Alloc(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu)
            {
                if (current_offset >= capacity)
                {
                    MOON_CORE_ASSERT(false, "Descriptor heap is full!");
                    return;
                }

                D3D12_CPU_DESCRIPTOR_HANDLE cpu = heap->GetCPUDescriptorHandleForHeapStart();
                D3D12_GPU_DESCRIPTOR_HANDLE gpu = heap->GetGPUDescriptorHandleForHeapStart();

                cpu.ptr += current_offset * descriptor_size;
                gpu.ptr += current_offset * descriptor_size;

                *out_cpu = cpu;
                *out_gpu = gpu;

                current_offset++;
            }

            void Reset()
            {
                current_offset = 0;
            }
        };

        static DescriptorAllocator s_allocator;
    };
};
