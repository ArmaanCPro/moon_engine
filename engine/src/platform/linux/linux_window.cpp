#include "moonpch.h"

#include "linux_window.h"

#include "moon/core/log.h"
#include "moon/events/application_event.h"
#include "moon/events/mouse_event.h"
#include "moon/events/key_event.h"

#include "platform/opengl/opengl_context.h"

#include <GLFW/glfw3.h>

namespace moon
{
    static bool s_glfw_initialized = false;

    static void glfw_error_callback(int error, const char* description)
    {
        MOON_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    window* window::create(const window_props& props)
    {
        return new linux_window(props);
    }

    linux_window::linux_window(const window_props& props)
    {
        init(props);
    }

    linux_window::~linux_window()
    {
        shutdown();
    }

    void linux_window::on_update()
    {
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    void linux_window::set_event_callback(const event_callback_fn& fn)
    {
        data_.event_callback = fn;
    }

    void linux_window::set_vsync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        data_.vsync = enabled;
    }

    void linux_window::init(const window_props& props)
    {
        data_.height = props.height;
        data_.width = props.width;
        data_.title = props.title;

        MOON_CORE_INFO("Creating window {0} ({1}, {2})", data_.title, data_.width, data_.height);

        if (!s_glfw_initialized)
        {
            int success = glfwInit();
            MOON_CORE_ASSERT(success, "Could not initialize GLFW!");
            glfwSetErrorCallback(glfw_error_callback);
            s_glfw_initialized = true;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

        window_ = glfwCreateWindow((int)props.width, (int)props.height, data_.title.c_str(), nullptr, nullptr);

        context_ = new opengl_context(window_);
        context_->init();

        glfwSetWindowUserPointer(window_, &data_);
        set_vsync(true);

        // glfw callbacks
        glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            data->width = width;
            data->height = height;
            window_resize_event event(width, height);
            data->event_callback(event);
        });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow* window)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            window_close_event event;
            data->event_callback(event);
        });

        glfwSetCharCallback(window_, [](GLFWwindow* window, unsigned int keycode)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            key_typed_event event(keycode);
            data->event_callback(event);
        });

        glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int, int action, int)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                key_pressed_event event(key, 0);
                data->event_callback(event);
            } break;
            case GLFW_RELEASE:
            {
                key_released_event event(key);
                data->event_callback(event);
            } break;
            case GLFW_REPEAT:
            {
                key_pressed_event event(key, 1);
                data->event_callback(event);
            } break;
            default:
                MOON_CORE_ERROR("Unknown key action");
            }
        });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int)
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

        glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            mouse_scrolled_event event((float)xoffset, (float)yoffset);
            data->event_callback(event);
        });

        glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos)
        {
            window_data* data = (window_data*)glfwGetWindowUserPointer(window);
            mouse_moved_event event((float)xpos, (float)ypos);
            data->event_callback(event);
        });
    }

    void linux_window::shutdown()
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}
