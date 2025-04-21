#include "moonpch.h"

#include "moon/core/input.h"

#include "linux_window.h"
#include "core/application.h"

#include <GLFW/glfw3.h>

namespace moon
{
    bool input::is_key_pressed(KeyCode key)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetKey(window, (int32_t)key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool input::is_mouse_button_pressed(MouseCode button)
    {
        auto* window = (GLFWwindow*)application::get().get_window().get_native_window();
        auto state = glfwGetMouseButton(window, (int32_t)button);
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
