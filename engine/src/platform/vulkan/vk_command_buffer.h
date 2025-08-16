#pragma once

#include "renderer/command_buffer.h"

#include "vk.h"

namespace moon::vulkan
{
    class vk_context;

    class vk_immediate_commands final
    {
    public:
        // max num of command buffers at a time, when full, we wait for a command buffer to become available
        static constexpr uint32_t s_max_command_buffers = 64;

        vk_immediate_commands(vk::Device device, uint32_t queue_family_index, const char* debug_name);
        ~vk_immediate_commands();
        vk_immediate_commands(const vk_immediate_commands&) = delete;
        vk_immediate_commands& operator=(const vk_immediate_commands&) = delete;

        struct command_buffer_wrapper
        {
            vk::CommandBuffer command_buffer = VK_NULL_HANDLE;
            vk::CommandBuffer command_buffer_allocated = VK_NULL_HANDLE;
            submit_handle handle = {};
            vk::Fence fence = VK_NULL_HANDLE;
            vk::Semaphore semaphore = VK_NULL_HANDLE;
            mutable bool is_encoding = false; // TODO use DOD to have 2 seperate vectors, one for ready to use cmdbufs and one for encoding/null cmdbufs
        };

        // returns current command buffer or creates one if it doesn't exist
        const command_buffer_wrapper& acquire();
        submit_handle submit(const command_buffer_wrapper& wrapper);
        void wait_semaphore(vk::Semaphore semaphore);
        void signal_semaphore(vk::Semaphore semaphore, uint64_t signal_value);
        vk::Semaphore acquire_last_submit_semaphore();
        vk::Fence get_vk_fence(submit_handle hdl) const;
        submit_handle get_last_submit_handle() const;
        submit_handle get_next_submit_handle() const;
        bool is_ready(submit_handle hdl, bool fast_check_no_vulkan = false) const;
        void wait(submit_handle hdl);
        void wait_all();

    private:
        void purge();

    private:
        // reference handles
        vk::Device m_device = VK_NULL_HANDLE;
        vk::Queue m_queue = VK_NULL_HANDLE;
        uint32_t m_queue_family_index = 0;

        vk::CommandPool m_command_pool = VK_NULL_HANDLE;
        const char* m_debug_name = "";
        std::array<command_buffer_wrapper, s_max_command_buffers> m_buffers;
        submit_handle m_last_submit_handle = {};
        submit_handle m_next_submit_handle = {};
        vk::SemaphoreSubmitInfo m_last_submit_semaphore = {{}, {}, vk::PipelineStageFlagBits2::eAllCommands };
        vk::SemaphoreSubmitInfo m_wait_semaphore = {{}, {}, vk::PipelineStageFlagBits2::eAllCommands };
        vk::SemaphoreSubmitInfo m_signal_semaphore = {{}, {}, vk::PipelineStageFlagBits2::eAllCommands };
        uint32_t m_num_available_command_buffers = s_max_command_buffers;
        uint32_t m_submit_counter = 1;
    };

    class vk_command_buffer final : public command_buffer
    {
    public:
        vk_command_buffer() = default;
        explicit vk_command_buffer(vk_context* context);
        ~vk_command_buffer() override;
        command_buffer& operator=(command_buffer&& other) = default;

        vk::CommandBuffer get_vk_command_buffer() const { return m_wrapper ? m_wrapper->command_buffer : VK_NULL_HANDLE; }
        operator vk::CommandBuffer() const { return get_vk_command_buffer(); }

        void transition_to_shader_read_only(texture_handle surface) const override;

        void cmd_bind_ray_tracing_pipeline(raytracing_pipeline_handle hdl) override;

        void cmd_bind_compute_pipeline(compute_pipeline_handle hdl) override;
        void cmd_dispatch_thread_groups(const dimensions& thread_group_count, const dependencies& deps) override;

        void cmd_push_debug_group_label(const char* label, uint32_t color_rgba) const override;
        void cmd_insert_debug_event_label(const char* label, uint32_t color_rgba) const override;
        void cmd_pop_debug_group_label() const override;

        void cmd_begin_rendering(const render_pass& r_pass, const framebuffer& fb, const dependencies& deps) override;
        void cmd_end_rendering() override;

        void cmd_bind_viewport(const viewport& viewport) override;
        void cmd_bind_scissor_rect(const scissor_rect& rect) override;

        void cmd_bind_render_pipeline(render_pipeline_handle hdl) override;
        void cmd_bind_depth_state(const depth_state& state) override;

        void cmd_bind_vertex_buffer(uint32_t index, buffer_handle buffer, uint64_t buffer_offset) override;
        void cmd_bind_index_buffer(buffer_handle index_buffer, IndexFormat index_format, uint64_t index_buffer_offset) override;
        void cmd_push_constants(const void* data, std::size_t size, std::size_t offset) override;

        void cmd_fill_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size, uint32_t data) override;
        void cmd_update_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size, const void* data) override;

        void cmd_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t base_instance) override;
        void cmd_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t base_instance) override;
        void cmd_draw_indirect(buffer_handle indirect_buffer, std::size_t offset, uint32_t draw_count, uint32_t stride) override;
        void cmd_draw_indexed_indirect(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset, std::size_t count_buffer_offset, uint32_t draw_count, uint32_t stride) override;
        void cmd_draw_indexed_indirect_count(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset, buffer_handle count_buffer, std::size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride) override;
        void cmd_draw_mesh_tasks(const dimensions& thread_group_count) override;
        void cmd_draw_mesh_tasks_indirect(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset, uint32_t draw_count, uint32_t stride) override;
        void cmd_draw_mesh_tasks_indirect_count(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset, buffer_handle count_buffer, std::size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride) override;
        void cmd_trace_rays(uint32_t width, uint32_t height, uint32_t depth, const dependencies& deps) override;

        void cmd_set_blend_color(const float color[4]) override;
        void cmd_set_depth_bias(float constant_factor, float clamp, float slope_factor) override;
        void cmd_set_depth_bias_enable(bool enable) override;

        void cmd_reset_query_pool(query_pool_handle pool, uint32_t first_query, uint32_t query_count) override;
        void cmd_write_timestamp(query_pool_handle pool, uint32_t query_index) override;

        void cmd_clear_color_image(texture_handle tex, const ClearColorValue& value, const texture_layers& layers) override;
        void cmd_copy_image(texture_handle src, texture_handle dst, const dimensions& extent, const offset3D& src_offset, const offset3D& dst_offset, const texture_layers& src_layers, const texture_layers& dst_layers) override;
        void cmd_generate_mipmap(texture_handle hdl) override;
        void cmd_update_tlas(accel_struct_handle hdl, buffer_handle instances_buffer) override;

    private:
        void use_compute_texture(texture_handle texture, vk::PipelineStageFlags2 dst_stage) const;
        void buffer_barrier(buffer_handle handle, vk::PipelineStageFlags2 src_stage, vk::PipelineStageFlags2 dst_stage) const;

    private:
        friend class vk_context;

        vk_context* m_context = nullptr;
        const vk_immediate_commands::command_buffer_wrapper* m_wrapper = nullptr;

        framebuffer m_framebuffer = {};
        submit_handle m_last_submit_handle = {};

        vk::Pipeline m_last_pipeline_bound = VK_NULL_HANDLE;

        bool m_is_rendering = false;

        render_pipeline_handle m_current_pipeline_graphics = {};
        compute_pipeline_handle m_current_pipeline_compute = {};
        raytracing_pipeline_handle m_current_pipeline_raytracing = {};
    };
}
