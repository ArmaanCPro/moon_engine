#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"
#include "vk_device.h"

#include "vk_swapchain.h"

#include "renderer/pool.h"
#include "renderer/handle.h"

namespace moon::vulkan
{
    struct render_pipeline_stage final
    {

    };
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

        device& get_device() override { return m_device; }
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


        pool<sampler_tag, vk::Sampler> m_samplers_pool;
        pool<buffer_tag, vk::Buffer> m_buffers_pool;
        pool<texture_tag, vk::Image> m_textures_pool;
        pool<query_pool_tag, vk::QueryPool> m_queries_pool;
    };
}
