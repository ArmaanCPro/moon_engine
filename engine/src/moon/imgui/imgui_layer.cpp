#include "moonpch.h"

#include <imgui.h>

//#ifndef MOON_IS_MONOLITHIC
// declared in moon.h, this function is to allow editor to access our context, useful for dll linking.
extern "C" MOON_API ImGuiContext* moon_get_imgui_context()
{
    MOON_PROFILE_FUNCTION();

    return ImGui::GetCurrentContext();
}
//#endif

#if 0
namespace moon
{
    imgui_layer::imgui_layer()
        :
        layer("ImGuiLayer")
    {}

    void imgui_layer::on_attach()
    {
        MOON_PROFILE_FUNCTION();

        IMGUI_CHECKVERSION();
        ImGuiContext* context = ImGui::CreateContext();
        if (!context) {
            MOON_CORE_ERROR("Failed to create ImGui context!");
        }
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

#ifdef WIN32
        ImGui_ImplWin32_Init((HWND)application::get().get_window().get_native_window());
        directx_context* dx_context = (directx_context*)application::get().get_window().get_context();
        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = dx_context->get_device().Get();
        init_info.CommandQueue = dx_context->get_command_queue().Get();
        init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
        init_info.NumFramesInFlight = dx_context->s_frames_in_flight;
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        // Descriptor heap for SRVs
        D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
        srv_heap_desc.NumDescriptors = 128;  // Change based on how many SRVs you expect to allocate.
        srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srv_heap_desc.NodeMask = 0;  // For multi-GPU (set to 0 if not applicable).

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap;
        HRESULT result = dx_context->get_device()->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&srv_descriptor_heap));
        MOON_CORE_ASSERT(SUCCEEDED(result), "Failed to create SRV descriptor heap");

        // Pointer to SRV descriptor heap
        init_info.SrvDescriptorHeap = srv_descriptor_heap.Get();

        static size_t current_offset = 0;
        static const size_t descriptor_size = dx_context->get_device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Lambda for allocating SRV descriptors
        init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle) {
            MOON_CORE_ASSERT(info && out_cpu_desc_handle && out_gpu_desc_handle, "Invalid descriptor allocation parameters!");

            // Check for overflow
            if (current_offset >= info->SrvDescriptorHeap->GetDesc().NumDescriptors) {
                MOON_CORE_ASSERT(false, "Exceeded SRV descriptor heap capacity!");
                return;
            }

            // Compute descriptor handles
            D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            cpu_handle.ptr += current_offset * descriptor_size;

            D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            gpu_handle.ptr += current_offset * descriptor_size;

            // Update output handles
            *out_cpu_desc_handle = cpu_handle;
            *out_gpu_desc_handle = gpu_handle;

            // Increment the offset for the next allocation
            current_offset++;
        };

        // Lambda for freeing SRV descriptors (optional, depending on use case)
        init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle) {
            // If you have a descriptor management system, free the specified descriptors here.
            // Alternatively, just allow growth if recycling isn't needed or managed manually.
        };
        ImGui_ImplDX12_Init(&init_info);
#else
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)(application::get().get_window().get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 460");
#endif
        MOON_CORE_TRACE("ImGui initialized");
    }

    void imgui_layer::on_detach()
    {
        MOON_PROFILE_FUNCTION();

#ifdef WIN32
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
#else
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
#endif
        ImGui::DestroyPlatformWindows();
        ImGui::DestroyContext();
        MOON_CORE_TRACE("ImGui shutdown");
    }

    void imgui_layer::on_event(event& e)
    {
        if (m_block_events_)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.handled |= e.is_in_category(EVENT_CATEGORY_MOUSE) && io.WantCaptureMouse;
            e.handled |= e.is_in_category(EVENT_CATEGORY_KEYBOARD) && io.WantCaptureKeyboard;
        }
    }

    void imgui_layer::begin()
    {
        MOON_PROFILE_FUNCTION();

#ifdef WIN32
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
#else
        if (glfwGetWindowAttrib((GLFWwindow*)application::get().get_window().get_native_window(), GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
#endif
        ImGui::NewFrame();
    }

    void imgui_layer::end()
    {
        MOON_PROFILE_FUNCTION();

        ImGuiIO& io = ImGui::GetIO();
        auto& app = application::get();
        io.DisplaySize = ImVec2((float)app.get_window().get_width(), (float)app.get_window().get_height());

        ImGui::Render();

#ifdef WIN32
        directx_context* dx_context = (directx_context*)application::get().get_window().get_context();
        dx_context->execute_command_list();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx_context->get_command_list().Get());
#else
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
#ifdef WIN32
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
#else
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
#endif
        }
    }
}

#endif
