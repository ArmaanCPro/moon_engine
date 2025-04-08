#include "moonpch.h"

#include "imgui_layer.h"

#include "core/application.h"
#include "core/events/application_event.h"
#include "core/events/key_event.h"
#include "core/events/mouse_event.h"

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

        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void imgui_layer::on_detach()
    {
        layer::on_detach();
    }

    void imgui_layer::on_update()
    {
        ImGuiIO& io = ImGui::GetIO();
        application& app = application::get();
        io.DisplaySize = ImVec2((float)app.get_window().get_width(), (float)app.get_window().get_height());

        float time = (float)glfwGetTime();
        io.DeltaTime = time_ > 0.0f ? (time - time_) : (1.0f / 240.0f);
        time_ = time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void imgui_layer::on_event(event& e)
    {
        static std::unordered_map<int, ImGuiMouseButton> mouse_map;
        mouse_map.insert( { GLFW_MOUSE_BUTTON_LEFT, ImGuiMouseButton_Left } );
        mouse_map.insert( { GLFW_MOUSE_BUTTON_RIGHT, ImGuiMouseButton_Right } );
        mouse_map.insert( { GLFW_MOUSE_BUTTON_MIDDLE, ImGuiMouseButton_Middle } );
        static std::unordered_map<int, ImGuiKey> key_map;
        key_map.insert( { GLFW_KEY_TAB, ImGuiKey_Tab } );
        key_map.insert( { GLFW_KEY_ESCAPE, ImGuiKey_Escape } );
        key_map.insert( { GLFW_KEY_SPACE, ImGuiKey_Space } );
        key_map.insert( { GLFW_KEY_ENTER, ImGuiKey_Enter } );
        key_map.insert( { GLFW_KEY_LEFT_SHIFT, ImGuiKey_LeftShift } );
        key_map.insert( { GLFW_KEY_LEFT_CONTROL, ImGuiKey_LeftCtrl } );
        key_map.insert( { GLFW_KEY_LEFT_ALT, ImGuiKey_LeftAlt } );
        key_map.insert( { GLFW_KEY_LEFT_SUPER, ImGuiKey_LeftSuper } );
        key_map.insert( { GLFW_KEY_BACKSPACE, ImGuiKey_Backspace } );
        key_map.insert( { GLFW_KEY_A, ImGuiKey_A } );

        if (!e.handled)
        {
            event_dispatcher dispatcher(e);
            dispatcher.dispatch<key_pressed_event>([this](key_pressed_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                if (key_map.contains(e.get_keycode()))
                    io.AddKeyEvent(key_map.at(e.get_keycode()), true);
                return false;
            });
            dispatcher.dispatch<key_released_event>([this](key_released_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddKeyEvent(key_map.at(e.get_keycode()), false);
                return false;
            });
            dispatcher.dispatch<key_typed_event>([this](key_typed_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                int keycode = e.get_keycode();
                if (keycode > 0 && keycode < 0x10000)
                    io.AddInputCharacter((unsigned short)keycode);
                return false;
            });
            dispatcher.dispatch<mouse_pressed_event>([this](mouse_pressed_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMouseButtonEvent(mouse_map.at(e.get_mouse_button()), true);
                return false;
            });
            dispatcher.dispatch<mouse_released_event>([this](mouse_released_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMouseButtonEvent(mouse_map.at(e.get_mouse_button()), false);
                return false;
            });
            dispatcher.dispatch<mouse_moved_event>([this](mouse_moved_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(e.get_x(), e.get_y());
                return false;
            });
            dispatcher.dispatch<mouse_scrolled_event>([this](mouse_scrolled_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMouseWheelEvent(e.get_x_offset(), e.get_y_offset());
                return false;
            });
            dispatcher.dispatch<window_resize_event>([this](window_resize_event& e)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)e.get_width(), (float)e.get_height());
                io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
                glViewport(0, 0, e.get_width(), e.get_height());
                return false;
            });
            dispatcher.dispatch<window_focus_event>([this](window_focus_event&)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddFocusEvent(true);
                return false;
            });
            dispatcher.dispatch<window_lost_focus_event>([this](window_lost_focus_event&)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddFocusEvent(false);
                return false;
            });
        }
    }
}
