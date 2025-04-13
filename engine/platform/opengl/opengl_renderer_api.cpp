#include "moonpch.h"
#include "opengl_renderer_api.h"

#include <glad/glad.h>

namespace moon
{
    void opengl_renderer_api::set_clear_color(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void opengl_renderer_api::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void opengl_renderer_api::draw_indexed(const std::shared_ptr<vertex_array>& vertex_array)
    {
        vertex_array->bind();
        glDrawElements(GL_TRIANGLES, vertex_array->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);
    }
}
