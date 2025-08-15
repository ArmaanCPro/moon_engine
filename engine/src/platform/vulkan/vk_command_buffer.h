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

    private:
        void use_compute_texture(texture_handle texture, vk::PipelineStageFlags2 dst_stage);
        void buffer_barrier(buffer_handle handle, vk::PipelineStageFlags2 src_stage, vk::PipelineStageFlags2 dst_stage);

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
