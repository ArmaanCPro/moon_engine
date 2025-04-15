#include "moonpch.h"
#include "renderer.h"

#include "render_command.h"
#include "camera.h"
#include "shader.h"
#include "moon/renderer/renderer2d.h"

namespace moon
{
    renderer::scene_data* renderer::s_scene_data_ = new renderer::scene_data;

    void renderer::init()
    {
        render_command::init();
        renderer2d::init();
    }

    void renderer::on_window_resize(uint32_t width, uint32_t height)
    {
        render_command::set_viewport(0, 0, width, height);
    }

    void renderer::begin_scene(const ortho_camera& cam)
    {
        s_scene_data_->view_projection_matrix = cam.get_view_projection_matrix();
    }

    void renderer::end_scene()
    {}

    void renderer::submit(const ref<shader>& shader, const ref<vertex_array>& vertex_array, const glm::mat4& transform)
    {
        shader->bind();
        shader->set_mat4("u_VP", s_scene_data_->view_projection_matrix);
        shader->set_mat4("u_Model", transform);

        vertex_array->bind();
        render_command::draw_indexed(vertex_array);
    }
}
