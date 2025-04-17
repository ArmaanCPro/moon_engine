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
    struct quad_vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 tex_coords;
        // TODO: texid
    };

    struct renderer2d_data
    {
        static constexpr uint32_t max_quads = 10000;
        static constexpr uint32_t max_vertices = max_quads * 4;
        static constexpr uint32_t max_indices = max_quads * 6;

        ref<vertex_array> quad_vertex_array;
        ref<vertex_buffer> quad_vertex_buffer;
        ref<shader> texture_shader;
        ref<texture2d> white_texture;

        uint32_t quad_index_count = 0;
        quad_vertex* quad_vertex_buffer_base = nullptr;
        quad_vertex* quad_vertex_buffer_ptr = nullptr;
    };

    static renderer2d_data s_data;

    void renderer2d::init()
    {
        MOON_PROFILE_FUNCTION();

        render_command::init();

        s_data.quad_vertex_array = moon::ref<vertex_array>(vertex_array::create());

        // vertex buffer
        s_data.quad_vertex_buffer = vertex_buffer::create(s_data.max_vertices * sizeof(quad_vertex));
        s_data.quad_vertex_buffer->set_layout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        s_data.quad_vertex_array->add_vertex_buffer(s_data.quad_vertex_buffer);

        s_data.quad_vertex_buffer_base = new quad_vertex[s_data.max_vertices];

        // index buffer
        uint32_t* quad_indices = new uint32_t[s_data.max_indices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_data.max_indices; i += 6)
        {
            quad_indices[i + 0] = offset + 0;
            quad_indices[i + 1] = offset + 1;
            quad_indices[i + 2] = offset + 2;

            quad_indices[i + 3] = offset + 2;
            quad_indices[i + 4] = offset + 3;
            quad_indices[i + 5] = offset + 0;

            offset += 4;
        }

        ref<index_buffer> quad_ib = index_buffer::create(quad_indices, s_data.max_indices);
        s_data.quad_vertex_array->set_index_buffer(quad_ib);
        delete[] quad_indices;

        // create a white shader used as a default texture
        s_data.white_texture = texture2d::create(1, 1);
        uint32_t white_texture_data = 0xffffffff;
        s_data.white_texture->set_data(&white_texture_data, sizeof(uint32_t));

        // create our texture shader
        s_data.texture_shader = shader::create("assets/shaders/texture.glsl");
        s_data.texture_shader->bind();
        s_data.texture_shader->set_int("u_Texture", 0);
    }

    void renderer2d::shutdown()
    {
        MOON_PROFILE_FUNCTION();


    }

    void renderer2d::begin_scene(const ortho_camera& camera)
    {
        MOON_PROFILE_FUNCTION();

        s_data.texture_shader->bind();
        s_data.texture_shader->set_mat4("u_VP", camera.get_view_projection_matrix());

        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;
    }

    void renderer2d::end_scene()
    {
        MOON_PROFILE_FUNCTION();

        uint32_t data_size = (uint32_t)((uint8_t*)s_data.quad_vertex_buffer_ptr - (uint8_t*)s_data.quad_vertex_buffer_base);
        s_data.quad_vertex_buffer->set_data(s_data.quad_vertex_buffer_base, data_size);

        flush();
    }

    void renderer2d::flush()
    {
        MOON_PROFILE_FUNCTION();

        s_data.quad_vertex_array->bind();
        render_command::draw_indexed(s_data.quad_vertex_array, s_data.quad_index_count);
        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;
    }

    void renderer2d::draw_quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        draw_quad({ position.x, position.y, 0.0f }, size, color);
    }

    void renderer2d::draw_quad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        MOON_PROFILE_FUNCTION();

        s_data.white_texture->bind();

        s_data.quad_vertex_buffer_ptr->position = position;
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coords = { 0.0f, 0.0f };
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = { position.x + size.x, position.y, 0.0f };
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coords = { 1.0f, 0.0f };
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = { position.x + size.x, position.y + size.y, 0.0f };
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coords = { 1.0f, 1.0f };
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = { position.x, position.y + size.y, 0.0f };
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coords = { 0.0f, 1.0f };
        s_data.quad_vertex_buffer_ptr++;

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        /*
        s_data.texture_shader->set_int("u_TilingFactor", 1.0f);
        s_data.white_texture->bind();

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_data.texture_shader->set_mat4("u_Model", transform);

        s_data.quad_vertex_array->bind();
        render_command::draw_indexed(s_data.quad_vertex_array);
        */
    }

    void renderer2d::draw_quad(const glm::vec2& position, const glm::vec2& size,
        const ref<texture2d>& texture, float tiling_factor, const glm::vec4& tint_color)
    {
        draw_quad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
    }

    void renderer2d::draw_quad(const glm::vec3& position, const glm::vec2& size,
        const ref<texture2d>& texture, float tiling_factor, const glm::vec4& tint_color)
    {
        MOON_PROFILE_FUNCTION();

        s_data.texture_shader->set_float4("u_Color", tint_color);
        s_data.texture_shader->set_float("u_TilingFactor", tiling_factor);
        texture->bind();

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_data.texture_shader->set_mat4("u_Model", transform);

        s_data.texture_shader->set_int("u_Texture", 0);

        s_data.quad_vertex_array->bind();
        render_command::draw_indexed(s_data.quad_vertex_array);
    }

    void renderer2d::draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation,
        const glm::vec4& color)
    {
        draw_rotated_quad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void renderer2d::draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation,
        const glm::vec4& color)
    {
        MOON_PROFILE_FUNCTION();

        s_data.texture_shader->set_float4("u_Color", color);
        s_data.white_texture->bind();

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_data.texture_shader->set_mat4("u_Model", transform);

        s_data.quad_vertex_array->bind();
        render_command::draw_indexed(s_data.quad_vertex_array);
    }

    void renderer2d::draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation,
        const ref<texture2d>& texture, float tiling_factor, const glm::vec4& tint_color)
    {
        draw_rotated_quad({ position.x, position.y, 0.0f }, size, rotation, texture, tiling_factor, tint_color);
    }

    void renderer2d::draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation,
        const ref<texture2d>& texture, float tiling_factor, const glm::vec4& tint_color)
    {
        MOON_PROFILE_FUNCTION();

        s_data.texture_shader->set_float4("u_Color", tint_color);
        s_data.texture_shader->set_float("u_TilingFactor", tiling_factor);
        texture->bind();

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_data.texture_shader->set_mat4("u_Model", transform);

        s_data.texture_shader->set_int("u_Texture", 0);

        s_data.quad_vertex_array->bind();
        render_command::draw_indexed(s_data.quad_vertex_array);
    }
}
