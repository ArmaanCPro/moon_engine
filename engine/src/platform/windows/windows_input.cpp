#include "moonpch.h"

#include "moon/core/input.h"

#include "windows_window.h"
#include "moon/core/application.h"
#include "glfw/glfw3.h"

namespace moon
{
    bool input::is_key_pressed(int keycode)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool input::is_mouse_button_pressed(int button)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> input::get_mouse_position()
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { (float)x, (float)y };
    }

    float input::get_mouse_x()
    {
        auto[x, y] = get_mouse_position();
        return x;
    }

    float input::get_mouse_y()
    {
        auto[x, y] = get_mouse_position();
        return y;
    }
}
