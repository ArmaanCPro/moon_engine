#include "moonpch.h"
#include "renderer2d.h"

#include "render_command.h"
#include "moon/renderer/shader.h"
#include "moon/renderer/buffer.h"
#include "moon/renderer/vertex_array.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace moon
{
    struct renderer2d_storage
    {
        ref<vertex_array> quad_vertex_array;
        ref<shader> quad_shader;
    };

    static renderer2d_storage* s_data;

    void renderer2d::init()
    {
        s_data = new renderer2d_storage;

        render_command::init();

        s_data->quad_vertex_array = moon::ref<vertex_array>(vertex_array::create());

        constexpr float square_verts[3 * 4] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        ref<vertex_buffer> square_vb = vertex_buffer::create(&square_verts[0], sizeof(square_verts));
        square_vb->set_layout({
            { ShaderDataType::Float3, "a_Pos" }
        });
        s_data->quad_vertex_array->add_vertex_buffer(square_vb);

        uint32_t square_indices[6] = {0, 1, 2, 2, 3, 0};
        ref<index_buffer> square_ib = index_buffer::create(&square_indices[0], sizeof(square_indices) / sizeof(uint32_t));
        s_data->quad_vertex_array->set_index_buffer(square_ib);

        s_data->quad_shader = shader::create("assets/shaders/flat_color.glsl");
    }

    void renderer2d::shutdown()
    {
        delete s_data;
    }

    void renderer2d::begin_scene(const ortho_camera& camera)
    {
        s_data->quad_shader->bind();
        s_data->quad_shader->set_mat4("u_VP", camera.get_view_projection_matrix());
    }

    void renderer2d::end_scene()
    {

    }

    void renderer2d::draw_quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        draw_quad({ position.x, position.y, 0.0f }, size, color);
    }

    void renderer2d::draw_quad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        s_data->quad_shader->bind();

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) /*  x Rotation */
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_data->quad_shader->set_mat4("u_Model", transform);
        s_data->quad_shader->set_float4("u_Color", color);

        s_data->quad_vertex_array->bind();
        render_command::draw_indexed(s_data->quad_vertex_array);
    }
}
