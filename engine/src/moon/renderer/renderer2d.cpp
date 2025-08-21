#include "moonpch.h"
#include "renderer2d.h"

#include "core/application.h"
#include "moon/renderer/shader.h"
#include "moon/renderer/buffer.h"
#include "vulkan/vk_context.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace moon
{
    struct quad_vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 tex_coords;
        float tex_index;
        float tiling_factor;
    };

    struct renderer2d_data
    {
        static constexpr uint32_t max_quads = 20000;
        static constexpr uint32_t max_vertices = max_quads * 4;
        static constexpr uint32_t max_indices = max_quads * 6;
        static constexpr uint32_t max_texture_slots = 32; // TODO: Render Capabilities

        ref<vertex_buffer> quad_vertex_buffer;
        ref<index_buffer> quad_index_buffer;
        ref<shader> texture_shader_vertex;
        ref<shader> texture_shader_fragment;
        ref<texture2d> white_texture;

        holder<render_pipeline_handle> quad_pipeline;

        uint32_t quad_index_count = 0;
        quad_vertex* quad_vertex_buffer_base = nullptr;
        quad_vertex* quad_vertex_buffer_ptr = nullptr;

        std::array<ref<texture2d>, max_texture_slots> texture_slots;
        uint32_t texture_slot_index = 1; // 0 = white texture

        glm::vec4 quad_vertex_positions[4];

        renderer2d::statistics stats;

        command_buffer* current_cmd;
        graphics_context* current_ctx;

        struct per_frame_data
        {
            uint32_t slot_to_desc[max_texture_slots]; // TODO remove the 32 limit altogether as we have bindless rendering
        }; // this is the limit for PC data, consider using descriptors
        per_frame_data frame_data;

        buffer_handle viewproj_buffer;
    };

    static renderer2d_data s_data;

    void renderer2d::init()
    {
        MOON_PROFILE_FUNCTION();

        // vertex buffer
        s_data.quad_vertex_buffer = vertex_buffer::create(s_data.max_vertices * sizeof(quad_vertex));
        s_data.quad_vertex_buffer->set_layout({
            .attributes = {
                {
                    { 0, 0, VertexFormat::Float3, 0 },
                    { 1, 0, VertexFormat::Float4, sizeof(glm::vec3) },
                    { 2, 0, VertexFormat::Float2, sizeof(glm::vec3) + sizeof(glm::vec4) },
                    { 3, 0, VertexFormat::Float1, sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2) },
                    { 4, 0, VertexFormat::Float1, sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2) + sizeof(float) }
                }
            },
            .bindings = {
                sizeof(quad_vertex)
            }
        });

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
        delete[] quad_indices;

        // create a white shader used as a default texture
        s_data.white_texture = texture2d::create(1, 1);
        uint32_t white_texture_data = 0xffffffff;
        s_data.white_texture->set_data(&white_texture_data, sizeof(uint32_t));

        int32_t samplers[s_data.max_texture_slots];
        for (uint32_t i = 0; i < s_data.max_texture_slots; i++)
        {
            samplers[i] = (int32_t)i;
        }

        // create our texture shader
        s_data.texture_shader_vertex = shader::create("assets/shaders/slang/texture.vert.spirv", ShaderStage::Vert);
        s_data.texture_shader_fragment = shader::create("assets/shaders/slang/texture.frag.spirv", ShaderStage::Frag);

        // set index 0 to white texture
        s_data.texture_slots[0] = s_data.white_texture;

        s_data.quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_data.quad_vertex_positions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_data.quad_vertex_positions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_data.quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        s_data.current_ctx = &application::get().get_context();
        s_data.quad_pipeline = s_data.current_ctx->create_render_pipeline({
            .vertex_input = s_data.quad_vertex_buffer->get_layout(),
            .sm_vert = s_data.texture_shader_vertex->get_handle(),
            .sm_frag = s_data.texture_shader_fragment->get_handle(),
            .color_attachments = { { s_data.current_ctx->get_swapchain_format() } },
            .depth_format = Format::Z_F32,
            .cull_mode = CullMode::Back
        }).value();

        s_data.viewproj_buffer = s_data.current_ctx->create_buffer({
            .usage = static_cast<uint8_t>(BufferUsageBits::Uniform),
            .storage_type = StorageType::Device,
            .size = sizeof(glm::mat4)
        }).value();
    }

    void renderer2d::shutdown()
    {
        MOON_PROFILE_FUNCTION();


    }

    void renderer2d::begin_scene(const camera& camera, const glm::mat4& transform)
    {
        MOON_PROFILE_FUNCTION();

        s_data.current_cmd = &s_data.current_ctx->acquire_command_buffer();

        glm::mat4 view_proj = camera.get_projection() * glm::inverse(transform);
        s_data.current_cmd->cmd_update_buffer(s_data.viewproj_buffer, &view_proj, sizeof(glm::mat4));

        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;

        s_data.texture_slot_index = 1;
    }

    void renderer2d::begin_scene(const ortho_camera& camera)
    {
        MOON_PROFILE_FUNCTION();

        s_data.current_cmd = &s_data.current_ctx->acquire_command_buffer();
        s_data.current_ctx->upload_buffer(s_data.viewproj_buffer, &camera.get_view_projection_matrix(), sizeof(glm::mat4));

        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;

        s_data.texture_slot_index = 1;
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

        // bind textures
        for (uint32_t i = 0; i < s_data.texture_slot_index; i++)
        {
            s_data.frame_data.slot_to_desc[i] = s_data.texture_slots[i]->get_handle().index();
        }

        render_pass rp = {
            .color = { { { .load_op = LoadOp::Clear, .clear_color = { 1.0f, 1.0f, 1.0f, 1.0f } } } },
        };
        framebuffer fb = {
            { { s_data.current_ctx->get_current_swapchain_texture() }}
        };

        s_data.current_cmd->cmd_begin_rendering(
            rp, fb
        );
        s_data.current_cmd->cmd_push_debug_group_label("Renderer2D::flush");
        s_data.current_cmd->cmd_bind_render_pipeline(s_data.quad_pipeline);
        s_data.current_cmd->cmd_push_constants(&s_data.frame_data);
        s_data.current_cmd->cmd_draw_indexed(s_data.quad_index_count, 1, 0, 0, 0);
        s_data.current_cmd->cmd_pop_debug_group_label();
        s_data.current_cmd->cmd_end_rendering();

        s_data.stats.draw_calls++;
    }

    void renderer2d::flush_and_reset()
    {
        end_scene();

        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;

        s_data.texture_slot_index = 1;
    }

    void renderer2d::draw_quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        draw_quad({ position.x, position.y, 0.0f }, size, color);
    }

    void renderer2d::draw_quad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        MOON_PROFILE_FUNCTION();

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::scale(transform, glm::vec3(size, 1.0f));

        draw_quad(transform, color);
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

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::scale(transform, glm::vec3(size, 1.0f));

        draw_quad(transform, texture, tiling_factor, tint_color);
    }

    void renderer2d::draw_quad(const glm::vec2& position, const glm::vec2& size, const ref<subtexture2d>& subtexture,
        float tiling_factor, const glm::vec4& tint_color)
    {
        draw_quad({ position.x, position.y, 0.0f }, size, subtexture, tiling_factor, tint_color);
    }

    void renderer2d::draw_quad(const glm::vec3& position, const glm::vec2& size, const ref<subtexture2d>& subtexture,
        float tiling_factor, const glm::vec4& tint_color)
    {
        MOON_PROFILE_FUNCTION();

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        float texindex = 0.0f;
        const glm::vec2* texture_coords = subtexture->get_texcoords();
        const ref<texture2d>& texture = subtexture->get_texture();

        // find the texture in the array of textures
        for (uint32_t i = 1; i < s_data.texture_slot_index; ++i)
        {
            if (*s_data.texture_slots[i].get() == *texture.get())
            {
                texindex = (float)i;
                break;
            }
        }

        // the texture was not in our array of textures
        if (texindex == 0.0f)
        {
            texindex = (float)s_data.texture_slot_index;
            s_data.texture_slots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::scale(transform, glm::vec3(size, 1.0f));

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texture_coords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

    void renderer2d::draw_quad(const glm::mat4& transform, const glm::vec4& color)
    {
        MOON_PROFILE_FUNCTION();

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        constexpr glm::vec2 texcoords[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        constexpr float texindex = 0.0f;

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texcoords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = 1.0f;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

    void renderer2d::draw_quad(const glm::mat4& transform, const ref<texture2d>& texture, float tiling_factor,
        const glm::vec4& tint_color)
    {
        MOON_PROFILE_FUNCTION();

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        float texindex = 0.0f;
        constexpr glm::vec2 texture_coords[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };

        // find the texture in the array of textures
        for (uint32_t i = 1; i < s_data.texture_slot_index; ++i)
        {
            if (*s_data.texture_slots[i].get() == *texture.get())
            {
                texindex = (float)i;
                break;
            }
        }

        // the texture was not in our array of textures
        if (texindex == 0.0f)
        {
            texindex = (float)s_data.texture_slot_index;
            s_data.texture_slots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texture_coords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
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

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        constexpr glm::vec2 texcoords[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        constexpr float texindex = 0.0f;
        constexpr float tiling_factor = 1.0f;

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::rotate(transform, rotation, glm::vec3(0, 0, 1))
            * glm::scale(transform, glm::vec3(size, 1.0f));

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texcoords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
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

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        constexpr glm::vec2 texcoords[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        float texindex = 0.0f;

        // find the texture in the array of textures
        for (uint32_t i = 1; i < s_data.texture_slot_index; ++i)
        {
            if (*s_data.texture_slots[i].get() == *texture.get())
            {
                texindex = (float)i;
                break;
            }
        }

        // the texture was not in our array of textures
        if (texindex == 0.0f)
        {
            texindex = (float)s_data.texture_slot_index;
            s_data.texture_slots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::rotate(transform, rotation, glm::vec3(0, 0, 1))
            * glm::scale(transform, glm::vec3(size, 1.0f));

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texcoords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

    void renderer2d::draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation,
        const ref<subtexture2d>& subtexture, float tiling_factor, const glm::vec4& tint_color)
    {
        draw_rotated_quad({ position.x, position.y, 0.0f }, size, rotation, subtexture, tiling_factor, tint_color);
    }

    void renderer2d::draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation,
        const ref<subtexture2d>& subtexture, float tiling_factor, const glm::vec4& tint_color)
    {
        MOON_PROFILE_FUNCTION();

                MOON_PROFILE_FUNCTION();

        if (s_data.quad_index_count >= renderer2d_data::max_indices)
            flush_and_reset();

        constexpr size_t quad_vertex_count = 4;
        float texindex = 0.0f;
        const glm::vec2* texture_coords = subtexture->get_texcoords();
        const ref<texture2d>& texture = subtexture->get_texture();

        // find the texture in the array of textures
        for (uint32_t i = 1; i < s_data.texture_slot_index; ++i)
        {
            if (*s_data.texture_slots[i].get() == *texture.get())
            {
                texindex = (float)i;
                break;
            }
        }

        // the texture was not in our array of textures
        if (texindex == 0.0f)
        {
            texindex = (float)s_data.texture_slot_index;
            s_data.texture_slots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position)
            * glm::rotate(transform, rotation, glm::vec3(0, 0, 1))
            * glm::scale(transform, glm::vec3(size, 1.0f));

        for (size_t i = 0; i < quad_vertex_count; i++)
        {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coords = texture_coords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texindex;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        // 4 vertices, but a quad has 6 indices
        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

    renderer2d::statistics renderer2d::get_stats()
    {
        return s_data.stats;
    }

    void renderer2d::reset_stats()
    {
        s_data.stats = {};
        //memset(&s_data.stats, 0, sizeof(statistics));
    }
}
