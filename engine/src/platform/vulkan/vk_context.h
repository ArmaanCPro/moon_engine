#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"
#include "vk_device.h"
#include "vk_pipelines.h"
#include "vk_render_types.h"

#include "vk_swapchain.h"
#include "vk_command_buffer.h"

#include "renderer/pool.h"
#include "renderer/handle.h"

#include <future>
#include <set>

namespace moon::vulkan
{
    static constexpr auto s_frames_in_flight = 2u;

    struct deferred_task_t
    {
        deferred_task_t(std::packaged_task<void()>&& task, submit_handle hdl)
            :
            m_task(std::move(task)), m_handle(hdl) {}
        std::packaged_task<void()> m_task;
        submit_handle m_handle;
    };

    struct frame_data
    {
        vk::UniqueCommandPool command_pool;
        vk::UniqueCommandBuffer command_buffer;

        vk::UniqueSemaphore swapchain_semaphore, render_semaphore;
        vk::UniqueFence render_fence;
    };

    class vk_context final : public graphics_context
    {
        static constexpr auto s_texture_binding = 0u;
        static constexpr auto s_sampler_binding = 1u;
        static constexpr auto s_storage_image_binding = 2u;
        static constexpr auto s_yuv_image_binding = 3u;
        static constexpr auto s_accel_struct_binding = 4u;
        static constexpr auto s_num_bindings = 5u;

    public:
        explicit vk_context(const native_handle& window);
        ~vk_context() override;

        vk_command_buffer& acquire_command_buffer() override;

        submit_handle submit(command_buffer& cmd, texture_handle present) override;
        void wait(submit_handle hdl) override;

        [[nodiscard]] std::expected<holder<buffer_handle>, result> create_buffer(const buffer_desc& desc, const char* debug_name = nullptr) override;
        [[nodiscard]] std::expected<holder<sampler_handle>, result> create_sampler(const sampler_state_desc& desc) override;
        [[nodiscard]] std::expected<holder<texture_handle>, result> create_texture(const texture_desc& desc, const char* debug_name = nullptr) override;
        [[nodiscard]] std::expected<holder<texture_handle>, result> create_texture_view(texture_handle hdl, const texture_view_desc& desc, const char* debug_name = nullptr) override;

        [[nodiscard]] std::expected<holder<compute_pipeline_handle>, result> create_compute_pipeline(const compute_pipeline_desc& desc) override;
        [[nodiscard]] std::expected<holder<render_pipeline_handle>, result> create_render_pipeline(const render_pipeline_desc& desc) override;
        [[nodiscard]] std::expected<holder<raytracing_pipeline_handle>, result> create_raytracing_pipeline(const ray_tracing_pipeline_desc& desc) override;
        [[nodiscard]] std::expected<holder<shader_module_handle>, result> create_shader_module(const shader_module_desc& desc) override;

        [[nodiscard]] std::expected<holder<query_pool_handle>, result> create_query_pool(uint32_t num_queries, const char* debug_name = nullptr) override;

        [[nodiscard]] std::expected<holder<accel_struct_handle>, result> create_accel_struct(const accel_struct_desc& desc) override;

        void destroy(compute_pipeline_handle hdl) override;
        void destroy(render_pipeline_handle hdl) override;
        void destroy(raytracing_pipeline_handle hdl) override;
        void destroy(shader_module_handle hdl) override;
        void destroy(sampler_handle hdl) override;
        void destroy(buffer_handle hdl) override;
        void destroy(texture_handle hdl) override;
        void destroy(query_pool_handle hdl) override;
        void destroy(accel_struct_handle hdl) override;
        void destroy(framebuffer& fb) override;

        uint64_t accel_struct_gpu_address(accel_struct_handle hdl) const override;

        result upload_buffer(buffer_handle hdl, const void* data, std::size_t size, std::size_t offset) override;
        result download_buffer(buffer_handle hdl, std::size_t size, void* out_data, std::size_t offset) override;
        uint8_t* get_mapped_ptr(buffer_handle hdl) const override;
        uint64_t gpu_address(buffer_handle hdl, std::size_t offset = 0) const override;
        void flush_mapped_memory(buffer_handle hdl, std::size_t offset, std::size_t size) const override;

        result upload_texture(texture_handle hdl, const texture_range_desc& range, const void* data) override;
        result download_texture(texture_handle hdl, const texture_range_desc& range, void* out_data) override;
        dimensions get_dimensions(texture_handle hdl) const override;
        float get_aspect_ratio(texture_handle hdl) const override;
        Format get_format(texture_handle hdl) const override;

        texture_handle get_current_swapchain_texture() override;
        Format get_swapchain_format() const override;
        uint32_t get_swapchain_image_count() const override;
        void recreate_swapchain(int new_width, int new_height) override;

        double get_timestamp_period_to_ms() const override;
        bool get_query_pool_results(query_pool_handle hdl, uint32_t first_query, uint32_t query_count, std::size_t stride, void* out_data) override;

        const vk_device& get_device() const { return m_device; }
        const vk_swapchain& get_swapchain() const { return *m_swapchain; }
        uint32_t get_swapchain_image_index() const { return m_swapchain_image_index; }

        vk::CommandBuffer get_active_command_buffer() { return get_current_frame().command_buffer.get(); }

        allocated_image& get_draw_image() { return m_draw_image; }
        allocated_image& get_depth_image() { return m_depth_image; }


        // VULKAN CONTEXT SPECIFIC
        vk::Pipeline get_vk_pipeline(compute_pipeline_handle hdl);
        vk::Pipeline get_vk_pipeline(render_pipeline_handle hdl);
        vk::Pipeline get_vk_pipeline(raytracing_pipeline_handle hdl);


        std::expected<shader_module_state, result> create_shader_module_from_spirv(const void* spirv, std::size_t num_bytes,
            const char* debug_name = nullptr) const;

        bool has_swapchain() const noexcept { return m_swapchain != nullptr; }

        void deferred_task(std::packaged_task<void()>&& task, submit_handle hdl = {}) const; // TODO: move deferred tasks to vk_device

        void check_and_update_descriptor_sets();
        void bind_default_descriptor_sets(vk::CommandBuffer cmd_buf, vk::PipelineBindPoint bind_point, vk::PipelineLayout layout) const;
    private:
        frame_data& get_current_frame() { return m_frames[m_frame_number]; }

        void recreate_swapchain();

        moon::result grow_descriptor_pool(uint32_t max_textures, uint32_t max_samplers, uint32_t max_accel_structs);
        // execute ready tasks
        void process_deferred_tasks() const;
        // wait for all tasks to be ready and execute them
        void wait_deferred_tasks();

        void generate_mipmap(texture_handle hdl);

    private:
        GLFWwindow* m_glfwwindow;

        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::UniqueSurfaceKHR m_surface;
        vk_device m_device;
        std::unique_ptr<vk_swapchain> m_swapchain;
        uint32_t m_swapchain_image_index = 0;

        std::array<frame_data, s_frames_in_flight> m_frames;
        uint32_t m_frame_number = 0;

        // offscreen draw and depth images
        allocated_image m_draw_image;
        allocated_image m_depth_image;
        float m_render_scale = 1.0f;

        bool m_resize_requested = false;

        // a texture/sampler was created since the last descriptor set update
        mutable bool m_awaiting_creation = false;
        mutable bool m_awaiting_new_immutable_samplers = false;

        vk_command_buffer m_current_command_buffer;
        mutable std::deque<deferred_task_t> m_deferred_tasks;

    public:
        vk::UniqueSemaphore m_timeline_semaphore;

        std::unique_ptr<vk_immediate_commands> m_immediate; // TODO move to device class

        uint32_t m_current_max_textures = 16;
        uint32_t m_current_max_samplers = 16;
        uint32_t m_current_max_accel_structs = 1;
        vk::UniqueDescriptorSetLayout m_vk_dsl;
        vk::UniqueDescriptorPool m_vk_dpool;
        vk::UniqueDescriptorSet m_vk_dset;

        // don't use staging on devices with shared host-visible memory
        bool m_use_staging = true;

        vk::PipelineCache m_pipeline_cache;

        texture_handle m_dummy_texture;

        // TODO: consider moving pools into a seperate class
        pool<shader_module_tag, shader_module_state> m_shader_modules_pool;
        pool<render_pipeline_tag, render_pipeline_state> m_render_pipelines_pool;
        pool<compute_pipeline_tag, compute_pipeline_state> m_compute_pipelines_pool;
        pool<raytracing_pipeline_tag, ray_tracing_pipeline_state> m_raytracing_pipelines_pool;
        pool<sampler_tag, vk::Sampler> m_samplers_pool;
        pool<buffer_tag, vulkan_buffer> m_buffers_pool;
        pool<texture_tag, vulkan_image> m_textures_pool;
        pool<query_pool_tag, vk::QueryPool> m_queries_pool;
        pool<accel_struct_tag, acceleration_structure> m_accel_structs_pool;
    };
}
