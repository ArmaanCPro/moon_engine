#include "moonpch.h"
#include "opengl_renderer_api.h"

#include "core/renderer/renderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>

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
        GLint id = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &id);
        glUniformMatrix4fv(glGetUniformLocation(id, "u_VP"), 1, GL_FALSE, glm::value_ptr(renderer::get_view_projection_matrix()));
        glUniformMatrix4fv(glGetUniformLocation(id, "u_Model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glDrawElements(GL_TRIANGLES, vertex_array->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);
    }
}
