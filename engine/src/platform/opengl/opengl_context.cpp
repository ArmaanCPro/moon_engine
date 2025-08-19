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
        MOON_CORE_ASSERT_MSG(window_handle, "Window handle is null!");
    }

}
