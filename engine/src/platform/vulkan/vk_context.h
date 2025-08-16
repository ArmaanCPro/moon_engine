#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"
#include "vk_device.h"
#include "vk_pipelines.h"
#include "vk_render_types.h"

#include "vk_swapchain.h"

#include "renderer/pool.h"
#include "renderer/handle.h"

namespace moon::vulkan
{
    static constexpr auto s_frames_in_flight = 2u;

    struct frame_data
    {
        vk::UniqueCommandPool command_pool;
        vk::UniqueCommandBuffer command_buffer;

        vk::UniqueSemaphore swapchain_semaphore, render_semaphore;
        vk::UniqueFence render_fence;
    };

    class vk_context final : public graphics_context
    {
    public:
        explicit vk_context(const native_handle& window);
        ~vk_context() override;

        void init() override;
        void swap_buffers() override;

        void begin_frame() override;
        void end_frame() override;

        vk_device& get_device() override { return m_device; }
        const vk_swapchain& get_swapchain() const { return m_swapchain; }
        uint32_t get_swapchain_image_index() const { return m_swapchain_image_index; }

        vk::CommandBuffer get_active_command_buffer() { return get_current_frame().command_buffer.get(); }

        allocated_image& get_draw_image() { return m_draw_image; }
        allocated_image& get_depth_image() { return m_depth_image; }

    private:
        frame_data& get_current_frame() { return m_frames[m_frame_number]; }

        void recreate_swapchain();

    private:
        GLFWwindow* m_glfwwindow;

        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::UniqueSurfaceKHR m_surface;
        vk_device m_device;
        vk_swapchain m_swapchain;
        uint32_t m_swapchain_image_index = 0;

        std::array<frame_data, s_frames_in_flight> m_frames;
        uint32_t m_frame_number = 0;

        // offscreen draw and depth images
        allocated_image m_draw_image;
        allocated_image m_depth_image;
        float m_render_scale = 1.0f;

        bool m_resize_requested = false;

    public:
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
