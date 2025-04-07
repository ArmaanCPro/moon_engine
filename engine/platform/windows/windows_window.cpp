#include "moonpch.h"

#include "windows_window.h"

#include "core/log.h"

namespace moon
{
    static bool s_glfw_initialized = false;

    window* window::create(const window_props& props)
    {
        return new windows_window(props);
    }

    windows_window::windows_window(const window_props& props)
    {
        init(props);
    }

    windows_window::~windows_window()
    {
        shutdown();
    }

    void windows_window::on_update()
    {
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    void windows_window::set_event_callback(const event_callback_fn& fn)
    {
        data_.event_callback = fn;
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
            MOON_CORE_ASSERT(success, "Could not initialize GLFW!");
            s_glfw_initialized = true;
        }

        window_ = glfwCreateWindow((int)props.width, (int)props.height, data_.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, &data_);
        set_vsync(true);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, (int)props.width, (int)props.height);
    }

    void windows_window::shutdown()
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}
