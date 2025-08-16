#pragma once

#include "renderer/handle.h"

#include "renderer/render_types.h"

namespace moon
{
    class command_buffer
    {
    public:
        virtual ~command_buffer() = default;

        virtual void transition_to_shader_read_only(texture_handle surface) const = 0;

        virtual void cmd_push_debug_group_label(const char* label, uint32_t color_rgba = 0xffffffff) const = 0;
        virtual void cmd_insert_debug_event_label(const char* label, uint32_t color_rgba = 0xffffffff) const = 0;
        virtual void cmd_pop_debug_group_label() const = 0;

        virtual void cmd_bind_ray_tracing_pipeline(raytracing_pipeline_handle hdl) = 0;

        virtual void cmd_bind_compute_pipeline(compute_pipeline_handle hdl) = 0;
        virtual void cmd_dispatch_thread_groups(const dimensions& thread_group_count, const dependencies& deps = {}) = 0;

        virtual void cmd_begin_rendering(const render_pass& r_pass, const framebuffer& desc, const dependencies& deps = {}) = 0;
        virtual void cmd_end_rendering() = 0;

        virtual void cmd_bind_viewport(const viewport& viewport) = 0;
        virtual void cmd_bind_scissor_rect(const scissor_rect& rect) = 0;

        virtual void cmd_bind_render_pipeline(render_pipeline_handle handle) = 0;
        virtual void cmd_bind_depth_state(const depth_state& state) = 0;

        virtual void cmd_bind_vertex_buffer(uint32_t index, buffer_handle buffer, uint64_t buffer_offset = 0) = 0;
        virtual void cmd_bind_index_buffer(buffer_handle index_buffer, IndexFormat index_format, uint64_t index_buffer_offset = 0) = 0;
        virtual void cmd_push_constants(const void* data, std::size_t size, std::size_t offset = 0) = 0;
        template <typename Struct>
        void cmd_push_constants(const Struct& data, std::size_t offset = 0)
        {
            this->cmd_push_constants(&data, sizeof(Struct), offset);
        }

        virtual void cmd_fill_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size, uint32_t data) = 0;
        virtual void cmd_update_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size, const void* data) = 0;
        template <typename Struct>
        void cmd_update_buffer(buffer_handle buffer, const Struct& data, std::size_t offset = 0)
        {
            this->cmd_update_buffer(buffer, offset, sizeof(Struct), &data);
        }

        virtual void cmd_draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t base_instance = 0) = 0;
        virtual void cmd_draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t base_instance = 0) = 0;
        virtual void cmd_draw_indirect(buffer_handle indirect_buffer, std::size_t offset, uint32_t draw_count, uint32_t stride = 0) = 0;
        virtual void cmd_draw_indexed_indirect(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset,
            std::size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride = 0) = 0;
        virtual void cmd_draw_indexed_indirect_count(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset,
            buffer_handle count_buffer, std::size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride = 0) = 0;
        virtual void cmd_draw_mesh_tasks(const dimensions& thread_group_count) = 0;
        virtual void cmd_draw_mesh_tasks_indirect(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset,
            uint32_t draw_count, uint32_t stride = 0) = 0;
        virtual void cmd_draw_mesh_tasks_indirect_count(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset,
            buffer_handle count_buffer, std::size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride = 0) = 0;
        virtual void cmd_trace_rays(uint32_t width, uint32_t height, uint32_t depth = 1, const dependencies& deps = {}) = 0;

        virtual void cmd_set_blend_color(const float color[4]) = 0;
        virtual void cmd_set_depth_bias(float constant_factor, float slope_factor, float clamp = 0.0f) = 0;
        virtual void cmd_set_depth_bias_enable(bool enable) = 0;

        virtual void cmd_reset_query_pool(query_pool_handle pool, uint32_t first_query, uint32_t query_count) = 0;
        virtual void cmd_write_timestamp(query_pool_handle pool, uint32_t query) = 0;

        virtual void cmd_clear_color_image(texture_handle tex, const ClearColorValue& value, const texture_layers& layers = {}) = 0;
        virtual void cmd_copy_image(texture_handle src, texture_handle dst, const dimensions& extent,
             const offset3D& src_offset = {}, const offset3D& dst_offset = {},
             const texture_layers& src_layers = {}, const texture_layers& dst_layers = {}) = 0;
        virtual void cmd_generate_mipmap(texture_handle handle) = 0;
        virtual void cmd_update_tlas(accel_struct_handle handle, buffer_handle instances_buffer) = 0;
    };
}
