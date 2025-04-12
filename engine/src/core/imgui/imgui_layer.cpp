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

namespace moon
{
    imgui_layer::imgui_layer()
        :
        layer("ImGuiLayer")
    {

    }

    imgui_layer::~imgui_layer()
    {

    }

    void imgui_layer::on_attach()
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)application::get().get_window().get_native_window(), true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void imgui_layer::on_detach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void imgui_layer::on_update()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
