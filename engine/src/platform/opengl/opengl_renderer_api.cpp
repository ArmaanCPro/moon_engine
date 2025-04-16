#include "moonpch.h"
#include "opengl_renderer_api.h"

#include "moon/renderer/renderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>

namespace moon
{
    void opengl_renderer_api::init()
    {
        MOON_PROFILE_FUNCTION();

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

    void opengl_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array)
    {
        glDrawElements(GL_TRIANGLES, vertex_array->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
