#include "moonpch.h"
#include "opengl_renderer_api.h"

#include "moon/renderer/renderer.h"
#include "renderer/graphics_context.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    void MOON_API OpenGLMessageCallback(
    [[maybe_unused]] unsigned source,
    [[maybe_unused]] unsigned type,
    [[maybe_unused]] unsigned id,
    [[maybe_unused]] unsigned severity,
    [[maybe_unused]] int length,
    [[maybe_unused]] const char* message,
    [[maybe_unused]] const void* userParam)
    {
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:         MOON_CORE_FATAL(message); return;
        case GL_DEBUG_SEVERITY_MEDIUM:       MOON_CORE_ERROR(message); return;
        case GL_DEBUG_SEVERITY_LOW:          MOON_CORE_WARN(message); return;
        case GL_DEBUG_SEVERITY_NOTIFICATION: MOON_CORE_TRACE(message); return;
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown severity level!");
    }

    void opengl_renderer_api::init(graphics_context* context)
    {
        MOON_PROFILE_FUNCTION();

#ifdef DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLMessageCallback, nullptr);

        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
    }

    void opengl_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    void opengl_renderer_api::set_clear_color(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void opengl_renderer_api::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void opengl_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {
        uint32_t count = index_count ? index_count : vertex_array->get_index_buffer()->get_count();
        glDrawElements(GL_TRIANGLES, (GLsizei)count, GL_UNSIGNED_INT, nullptr);
        //glBindTexture(GL_TEXTURE_2D, 0);
    }
}
