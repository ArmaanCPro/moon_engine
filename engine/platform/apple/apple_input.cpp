#include "moonpch.h"

#include "apple_input.h"

#include "apple_window.h"
#include "core/application.h"

#include <GLFW/glfw3.h>

namespace moon
{
    input* input::s_instance = new apple_input();

    bool apple_input::is_key_pressed_impl(int keycode)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool apple_input::is_mouse_button_pressed_impl(int button)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> apple_input::get_mouse_position_impl()
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { (float)x, (float)y };
    }

    float apple_input::get_mouse_x_impl()
    {
        auto[x, y] = get_mouse_position_impl();
        return x;
    }

    float apple_input::get_mouse_y_impl()
    {
        auto[x, y] = get_mouse_position_impl();
        return y;
    }
}
