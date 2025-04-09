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
        mouse_map.insert({MOON_MOUSE_BUTTON_LEFT, ImGuiMouseButton_Left});
        mouse_map.insert({MOON_MOUSE_BUTTON_RIGHT, ImGuiMouseButton_Right});
        mouse_map.insert({MOON_MOUSE_BUTTON_MIDDLE, ImGuiMouseButton_Middle});

        static std::unordered_map<int, ImGuiKey> key_map;
        key_map.insert({MOON_KEY_TAB, ImGuiKey_Tab});
        key_map.insert({MOON_KEY_ESCAPE, ImGuiKey_Escape});
        key_map.insert({MOON_KEY_SPACE, ImGuiKey_Space});
        key_map.insert({MOON_KEY_ENTER, ImGuiKey_Enter});
        key_map.insert({MOON_KEY_LEFT_SHIFT, ImGuiKey_LeftShift});
        key_map.insert({MOON_KEY_LEFT_CONTROL, ImGuiKey_LeftCtrl});
        key_map.insert({MOON_KEY_LEFT_ALT, ImGuiKey_LeftAlt});
        key_map.insert({MOON_KEY_LEFT_SUPER, ImGuiKey_LeftSuper});
        key_map.insert({MOON_KEY_RIGHT_SHIFT, ImGuiKey_RightShift});
        key_map.insert({MOON_KEY_RIGHT_CONTROL, ImGuiKey_RightCtrl});
        key_map.insert({MOON_KEY_RIGHT_ALT, ImGuiKey_RightAlt});
        key_map.insert({MOON_KEY_RIGHT_SUPER, ImGuiKey_RightSuper});
        key_map.insert({MOON_KEY_BACKSPACE, ImGuiKey_Backspace});
        key_map.insert({MOON_KEY_INSERT, ImGuiKey_Insert});
        key_map.insert({MOON_KEY_DELETE, ImGuiKey_Delete});
        key_map.insert({MOON_KEY_HOME, ImGuiKey_Home});
        key_map.insert({MOON_KEY_END, ImGuiKey_End});
        key_map.insert({MOON_KEY_PAGE_UP, ImGuiKey_PageUp});
        key_map.insert({MOON_KEY_PAGE_DOWN, ImGuiKey_PageDown});
        key_map.insert({MOON_KEY_UP, ImGuiKey_UpArrow});
        key_map.insert({MOON_KEY_DOWN, ImGuiKey_DownArrow});
        key_map.insert({MOON_KEY_LEFT, ImGuiKey_LeftArrow});
        key_map.insert({MOON_KEY_RIGHT, ImGuiKey_RightArrow});
        key_map.insert({MOON_KEY_F1, ImGuiKey_F1});
        key_map.insert({MOON_KEY_F2, ImGuiKey_F2});
        key_map.insert({MOON_KEY_F3, ImGuiKey_F3});
        key_map.insert({MOON_KEY_F4, ImGuiKey_F4});
        key_map.insert({MOON_KEY_F5, ImGuiKey_F5});
        key_map.insert({MOON_KEY_F6, ImGuiKey_F6});
        key_map.insert({MOON_KEY_F7, ImGuiKey_F7});
        key_map.insert({MOON_KEY_F8, ImGuiKey_F8});
        key_map.insert({MOON_KEY_F9, ImGuiKey_F9});
        key_map.insert({MOON_KEY_F10, ImGuiKey_F10});
        key_map.insert({MOON_KEY_F11, ImGuiKey_F11});
        key_map.insert({MOON_KEY_F12, ImGuiKey_F12});
        key_map.insert({MOON_KEY_A, ImGuiKey_A});
        key_map.insert({MOON_KEY_B, ImGuiKey_B});
        key_map.insert({MOON_KEY_C, ImGuiKey_C});
        key_map.insert({MOON_KEY_D, ImGuiKey_D});
        key_map.insert({MOON_KEY_E, ImGuiKey_E});
        key_map.insert({MOON_KEY_F, ImGuiKey_F});
        key_map.insert({MOON_KEY_G, ImGuiKey_G});
        key_map.insert({MOON_KEY_H, ImGuiKey_H});
        key_map.insert({MOON_KEY_I, ImGuiKey_I});
        key_map.insert({MOON_KEY_J, ImGuiKey_J});
        key_map.insert({MOON_KEY_K, ImGuiKey_K});
        key_map.insert({MOON_KEY_L, ImGuiKey_L});
        key_map.insert({MOON_KEY_M, ImGuiKey_M});
        key_map.insert({MOON_KEY_N, ImGuiKey_N});
        key_map.insert({MOON_KEY_O, ImGuiKey_O});
        key_map.insert({MOON_KEY_P, ImGuiKey_P});
        key_map.insert({MOON_KEY_Q, ImGuiKey_Q});
        key_map.insert({MOON_KEY_R, ImGuiKey_R});
        key_map.insert({MOON_KEY_S, ImGuiKey_S});
        key_map.insert({MOON_KEY_T, ImGuiKey_T});
        key_map.insert({MOON_KEY_U, ImGuiKey_U});
        key_map.insert({MOON_KEY_V, ImGuiKey_V});
        key_map.insert({MOON_KEY_W, ImGuiKey_W});
        key_map.insert({MOON_KEY_X, ImGuiKey_X});
        key_map.insert({MOON_KEY_Y, ImGuiKey_Y});
        key_map.insert({MOON_KEY_Z, ImGuiKey_Z});
        key_map.insert({MOON_KEY_0, ImGuiKey_0});
        key_map.insert({MOON_KEY_1, ImGuiKey_1});
        key_map.insert({MOON_KEY_2, ImGuiKey_2});
        key_map.insert({MOON_KEY_3, ImGuiKey_3});
        key_map.insert({MOON_KEY_4, ImGuiKey_4});
        key_map.insert({MOON_KEY_5, ImGuiKey_5});
        key_map.insert({MOON_KEY_6, ImGuiKey_6});
        key_map.insert({MOON_KEY_7, ImGuiKey_7});
        key_map.insert({MOON_KEY_8, ImGuiKey_8});
        key_map.insert({MOON_KEY_9, ImGuiKey_9});
        key_map.insert({MOON_KEY_GRAVE_ACCENT, ImGuiKey_GraveAccent});
        key_map.insert({MOON_KEY_MINUS, ImGuiKey_Minus});
        key_map.insert({MOON_KEY_EQUAL, ImGuiKey_Equal});
        key_map.insert({MOON_KEY_LEFT_BRACKET, ImGuiKey_LeftBracket});
        key_map.insert({MOON_KEY_RIGHT_BRACKET, ImGuiKey_RightBracket});
        key_map.insert({MOON_KEY_BACKSLASH, ImGuiKey_Backslash});
        key_map.insert({MOON_KEY_SEMICOLON, ImGuiKey_Semicolon});
        key_map.insert({MOON_KEY_APOSTROPHE, ImGuiKey_Apostrophe});
        key_map.insert({MOON_KEY_COMMA, ImGuiKey_Comma});
        key_map.insert({MOON_KEY_PERIOD, ImGuiKey_Period});
        key_map.insert({MOON_KEY_SLASH, ImGuiKey_Slash});

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
