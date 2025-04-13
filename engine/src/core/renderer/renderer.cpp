#include "moonpch.h"
#include "renderer.h"

#include "render_command.h"
#include "camera.h"
#include "shader.h"

namespace moon
{
    renderer::scene_data* renderer::s_scene_data_ = new renderer::scene_data;

    void renderer::begin_scene(const ortho_camera& cam)
    {
        s_scene_data_->view_projection_matrix = cam.get_view_projection_matrix();
    }

    void renderer::end_scene()
    {}

    void renderer::submit(const std::shared_ptr<shader>& shader, const std::shared_ptr<vertex_array>& vertex_array)
    {
        shader->bind();
        shader->upload_uniform_mat4("u_VP", s_scene_data_->view_projection_matrix);

        vertex_array->bind();
        render_command::draw_indexed(vertex_array);
    }
}
