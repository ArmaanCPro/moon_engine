#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"
#include "vk_device.h"

#include "vk_swapchain.h"

namespace moon
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
        vk_context(const native_handle& window);
        ~vk_context();

        void init() override;
        void swap_buffers() override;

        void begin_frame() override;
        void end_frame() override;

        device& get_device() override { return m_device; }

        vk::CommandBuffer get_active_command_buffer() { return get_current_frame().command_buffer.get(); }

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

        bool m_resize_requested = false;
    };
}
