#include "moonpch.h"

#include "windows_window.h"

#include "core/log.h"

namespace moon
{
    static bool s_glfw_initialized = false;

    moon::windows_window::windows_window(const window_props& props)
    {
        init(props);
    }

    windows_window::~windows_window()
    {
        shutdown();
    }

    window* window::create(const window_props& props)
    {
        return new windows_window(props);
    }

    void windows_window::set_vsync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        data_.vsync = enabled;
    }

    void windows_window::init(const window_props& props)
    {
        data_.height = props.height;
        data_.width = props.width;
        data_.title = props.title;

        MOON_CORE_INFO("Creating window {0} ({1}, {2})", data_.title, data_.width, data_.height);

        if (!s_glfw_initialized)
        {
            int success = glfwInit();
            MOON_CORE_WARN("glfw status: {0}", success);
            //MOON_CORE_ASSERT(success, "Could not initialize GLFW!");
            s_glfw_initialized = true;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window_ = glfwCreateWindow((int)data_.width, (int)data_.height, data_.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, &data_);
        set_vsync(true);
    }

    void windows_window::shutdown()
    {
        glfwDestroyWindow(window_);
    }
}
