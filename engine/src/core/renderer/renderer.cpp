#include "moonpch.h"
#include "renderer.h"

#include "render_command.h"
#include "camera.h"
#include "shader.h"
#include "opengl/opengl_shader.h"

namespace moon
{
    renderer::scene_data* renderer::s_scene_data_ = new renderer::scene_data;

    void renderer::begin_scene(const ortho_camera& cam)
    {
        s_scene_data_->view_projection_matrix = cam.get_view_projection_matrix();
    }

    void renderer::end_scene()
    {}

    void renderer::submit(const ref<shader>& shader, const ref<vertex_array>& vertex_array, const glm::mat4& transform)
    {
        shader->bind();
        std::dynamic_pointer_cast<opengl_shader>(shader)->upload_uniform_mat4("u_VP", s_scene_data_->view_projection_matrix);
        std::dynamic_pointer_cast<opengl_shader>(shader)->upload_uniform_mat4("u_Model", transform);

        vertex_array->bind();
        render_command::draw_indexed(vertex_array);
    }
}
