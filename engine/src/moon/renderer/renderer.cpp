#include "moonpch.h"
#include "renderer.h"

#include "buffer.h"
#include "camera.h"
#include "shader.h"
#include "moon/renderer/renderer2d.h"

namespace moon
{
    renderer_api renderer::s_api_ = renderer_api{ renderer_api::API::Vulkan };

    renderer::scene_data* renderer::s_scene_data_ = nullptr;

    void renderer::init()
    {
        MOON_PROFILE_FUNCTION();

        s_scene_data_ = new scene_data;

        renderer2d::init();
    }

    void renderer::shutdown()
    {
        MOON_PROFILE_FUNCTION();

        delete s_scene_data_;
        renderer2d::shutdown();
    }

    void renderer::on_window_resize(uint32_t width, uint32_t height)
    {
    }

    void renderer::begin_scene(const ortho_camera& cam)
    {
        s_scene_data_->view_projection_matrix = cam.get_view_projection_matrix();
    }

    void renderer::end_scene()
    {}

    void renderer::submit(const ref<shader>& shader, const ref<vertex_buffer>& vert_buffer, const ref<index_buffer>& index_buffer, const glm::mat4& transform)
    {
        shader->bind();
        shader->set_mat4("u_VP", s_scene_data_->view_projection_matrix);
        shader->set_mat4("u_Model", transform);

        vert_buffer->bind();
        //render_command::draw_indexed(vertex_array);
    }
}
