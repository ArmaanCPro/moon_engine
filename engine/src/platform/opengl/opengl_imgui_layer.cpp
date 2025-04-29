#include "moonpch.h"
#include "opengl_imgui_layer.h"

#include "core/application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace moon
{
    opengl_imgui_layer::opengl_imgui_layer()
    {}

    opengl_imgui_layer::~opengl_imgui_layer()
    {}

    void opengl_imgui_layer::on_attach()
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

        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)(application::get().get_window().get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 460");
        MOON_CORE_TRACE("ImGui initialized");
    }

    void opengl_imgui_layer::on_detach()
    {
        MOON_PROFILE_FUNCTION();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyPlatformWindows();
        ImGui::DestroyContext();
        MOON_CORE_TRACE("ImGui shutdown");
    }

    void opengl_imgui_layer::on_imgui_render()
    {
        imgui_layer::on_imgui_render();
    }

    void opengl_imgui_layer::on_event(event& e)
    {
        if (m_block_events_)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.handled |= e.is_in_category(EVENT_CATEGORY_MOUSE) && io.WantCaptureMouse;
            e.handled |= e.is_in_category(EVENT_CATEGORY_KEYBOARD) && io.WantCaptureKeyboard;
        }
    }

    void opengl_imgui_layer::begin()
    {
        MOON_PROFILE_FUNCTION();

        if (glfwGetWindowAttrib((GLFWwindow*)application::get().get_window().get_native_window(), GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void opengl_imgui_layer::end()
    {
        MOON_PROFILE_FUNCTION();

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
