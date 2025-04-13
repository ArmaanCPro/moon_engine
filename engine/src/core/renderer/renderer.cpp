#include "moonpch.h"
#include "renderer.h"

#include "render_command.h"
#include "camera.h"

namespace moon
{
    glm::mat4 renderer::s_vp_matrix_ = glm::mat4(1.0f);

    void renderer::begin_scene(const camera& cam)
    {
        s_vp_matrix_ = cam.get_view_projection_matrix();
    }

    void renderer::end_scene()
    {}

    void renderer::submit(const std::shared_ptr<vertex_array>& vertex_array)
    {
        render_command::draw_indexed(vertex_array);
    }
}
