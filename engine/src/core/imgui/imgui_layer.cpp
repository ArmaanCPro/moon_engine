#include "moonpch.h"

#include "imgui_layer.h"

#include "core/application.h"
#include "core/events/application_event.h"
#include "core/events/key_event.h"
#include "core/events/mouse_event.h"

#include "core/core.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

// declared in moon.h, this function is to allow editor to access our context, useful for dll linking.
extern "C" MOON_API ImGuiContext* moon_get_imgui_context()
{
    return ImGui::GetCurrentContext();
}

namespace moon
{
    imgui_layer::imgui_layer()
        :
        layer("ImGuiLayer")
    {}

    imgui_layer::~imgui_layer()
    {}

    void imgui_layer::on_attach()
    {
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

        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)(application::get().get_window().get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 460");
        MOON_CORE_TRACE("ImGui initialized");
    }

    void imgui_layer::on_detach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyPlatformWindows();
        ImGui::DestroyContext();
        MOON_CORE_TRACE("ImGui shutdown");
    }

    void imgui_layer::on_imgui_render()
    {
        if (!ImGui::GetCurrentContext())
        {
            MOON_CORE_ERROR("No ImGui context active, skipping rendering");
            return;
        }

        static bool show = false;
        ImGui::ShowDemoWindow(&show);
    }

    void imgui_layer::begin()
    {
        if (glfwGetWindowAttrib((GLFWwindow*)application::get().get_window().get_native_window(), GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void imgui_layer::end()
    {
        ImGuiIO& io = ImGui::GetIO();
        auto& app = application::get();
        io.DisplaySize = ImVec2((float)app.get_window().get_width(), (float)app.get_window().get_height());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}
