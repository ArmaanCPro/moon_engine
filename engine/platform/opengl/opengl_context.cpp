#include "moonpch.h"
#include "opengl_context.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace moon
{
    opengl_context::opengl_context(GLFWwindow* window_handle)
        :
        window_handle_(window_handle)
    {
        MOON_CORE_ASSERT(window_handle, "Window handle is null!");
    }

    void opengl_context::init()
    {
        glfwMakeContextCurrent(window_handle_);
        // setup glad
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        MOON_CORE_ASSERT(status, "Failed to initialize Glad");
    }

    void opengl_context::swap_buffers()
    {
        glfwSwapBuffers(window_handle_);
    }
}
