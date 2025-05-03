#include "moonpch.h"
#include "directx_imgui_layer.h"
#include "directx_context.h"
#include "moon/core/application.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

// Forward declare ImGui Win32 handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace moon
{
    directx_imgui_layer::DescriptorAllocator directx_imgui_layer::s_allocator;

    directx_imgui_layer::directx_imgui_layer()
        : imgui_layer()
    {
        MOON_PROFILE_FUNCTION();
    }

    directx_imgui_layer::~directx_imgui_layer()
    {
        MOON_PROFILE_FUNCTION();
    }

    void directx_imgui_layer::on_attach()
    {
        MOON_PROFILE_FUNCTION();

        // Create ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup style
        ImGui::StyleColorsDark();

        // Get DirectX context
        auto& app = application::get();
        auto* dx_context = static_cast<directx_context*>(app.get_context());
        
        // Create descriptor heap for ImGui
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 128; // Adjust this as needed for your app
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        
        HRESULT hr = dx_context->get_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srv_heap));
        MOON_CORE_ASSERT(SUCCEEDED(hr), "Failed to create ImGui descriptor heap!");
        
        // Setup allocator
        s_allocator.heap = m_srv_heap.Get();
        s_allocator.capacity = desc.NumDescriptors;
        s_allocator.descriptor_size = dx_context->get_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        s_allocator.current_offset = 0;
        
        // Setup Win32 backend
        HWND hwnd = static_cast<HWND>(app.get_window().get_native_handle());
        ImGui_ImplWin32_Init(hwnd);
        
        // Setup DirectX backend
        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = dx_context->get_device().Get();
        init_info.CommandQueue = dx_context->get_command_queue().Get();
        init_info.NumFramesInFlight = dx_context->s_frames_in_flight;
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        init_info.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        init_info.SrvDescriptorHeap = m_srv_heap.Get();
        init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
            s_allocator.Alloc(info->Device, out_cpu, out_gpu);
        };
        init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu)
        {
            // no freeing of desciptors
        };
        
        bool result = ImGui_ImplDX12_Init(&init_info);
        MOON_CORE_ASSERT(result, "Failed to initialize ImGui DX12 backend!");
        
        m_initialized = true;
    }

    void directx_imgui_layer::on_detach()
    {
        MOON_PROFILE_FUNCTION();
        
        // Shutdown backends
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        
        // Release resources
        if (m_srv_heap)
        {
            m_srv_heap->Release();
            m_srv_heap = nullptr;
        }
        
        m_initialized = false;
    }

    void directx_imgui_layer::begin()
    {
        MOON_PROFILE_FUNCTION();
        
        MOON_CORE_ASSERT(m_initialized, "ImGui layer is not initialized!");
        
        // Begin new frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void directx_imgui_layer::end()
    {
        MOON_PROFILE_FUNCTION();

        MOON_CORE_ASSERT(m_initialized, "ImGui layer is not initialized!");

        // Finalize frame
        ImGui::Render();
        
        // Get context
        auto& app = application::get();
        auto* dx_context = dynamic_cast<directx_context*>(app.get_context());
        
        ComPtr<ID3D12GraphicsCommandList> command_list = dx_context->get_command_list();
        
        // IMPORTANT: Set descriptor heaps BEFORE rendering ImGui
        ID3D12DescriptorHeap* heaps[] = { m_srv_heap.Get() };
        command_list->SetDescriptorHeaps(1, heaps);
        
        // Render ImGui
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());
        
        // Update viewports (if enabled)
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(nullptr, (void*)command_list.Get());
        }
    }

    void directx_imgui_layer::on_imgui_render()
    {
        imgui_layer::on_imgui_render();
    }

    void directx_imgui_layer::on_event(event& e)
    {
        if (m_block_events_)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.handled |= e.is_in_category(EVENT_CATEGORY_MOUSE) && io.WantCaptureMouse;
            e.handled |= e.is_in_category(EVENT_CATEGORY_KEYBOARD) && io.WantCaptureKeyboard;
        }
    }
}
