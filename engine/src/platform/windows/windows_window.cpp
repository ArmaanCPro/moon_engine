#include "moonpch.h"

#include "windows_window.h"

#include "moon/core/log.h"
#include "moon/events/application_event.h"
#include "moon/events/mouse_event.h"
#include "moon/events/key_event.h"

#include <GLFW/glfw3.h>

namespace moon
{
    static int s_glfw_window_count = 0;

    static void glfw_error_callback(int error, const char* description)
    {
        MOON_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    scope<window> window::create(const window_props& props)
    {
        return create_scope<windows_window>(props);
    }

    windows_window::windows_window(const window_props& props)
    {
        MOON_PROFILE_FUNCTION();

        data_.height = props.height;
        data_.width = props.width;
        data_.title = props.title;

        MOON_CORE_INFO("Creating window {0} ({1}, {2})", data_.title, data_.width, data_.height);

        if (s_glfw_window_count == 0)
        {
            MOON_PROFILE_SCOPE("glfw init");
            int success = glfwInit();
            MOON_CORE_ASSERT(success, "Could not initialize GLFW!");
            glfwSetErrorCallback(glfw_error_callback);
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        {
            MOON_PROFILE_SCOPE("glfwCreateWindow");
            window_ = glfwCreateWindow((int)props.width, (int)props.height, data_.title.c_str(), nullptr, nullptr);
            ++s_glfw_window_count;
        }

        glfwSetWindowUserPointer(window_, &data_);
        windows_window::set_vsync(true);

        handle_.type = NativeHandleType::GLFW;
        handle_.handle = window_;

        set_glfw_callbacks(window_);
    }

    windows_window::~windows_window()
    {
        MOON_PROFILE_FUNCTION();

        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void windows_window::on_update()
    {
        MOON_PROFILE_FUNCTION();

        glfwPollEvents();
    }

    void windows_window::set_event_callback(const event_callback_fn& fn)
    {
        data_.event_callback = fn;
    }

    void windows_window::set_vsync(bool enabled)
    {
        MOON_PROFILE_FUNCTION();

        // no way to easily set vsync in vulkan
    }

    void windows_window::set_glfw_callbacks(GLFWwindow* window)
    {
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            data->width = width;
            data->height = height;
            data->framebufferResized = true;
            window_resize_event event(width, height);
            data->event_callback(event);
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            window_close_event event;
            data->event_callback(event);
        });

        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int keycode)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            key_typed_event event(keycode);
            data->event_callback(event);
        });

        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                key_pressed_event event(key, -1);
                data->event_callback(event);
            } break;
            case GLFW_RELEASE:
            {
                key_released_event event(key);
                data->event_callback(event);
            } break;
            case GLFW_REPEAT:
            {
                key_pressed_event event(key, 0);
                data->event_callback(event);
            } break;
            default:
                MOON_CORE_ERROR("Unknown key action");
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                mouse_pressed_event event(button);
                data->event_callback(event);
            } break;
            case GLFW_RELEASE:
            {
                mouse_released_event event(button);
                data->event_callback(event);
            } break;
            default:
                MOON_CORE_ERROR("Unknown mouse action");
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            mouse_scrolled_event event((float)xoffset, (float)yoffset);
            data->event_callback(event);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            mouse_moved_event event((float)xpos, (float)ypos);
            data->event_callback(event);
        });
    }
}
