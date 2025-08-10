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

    private:
        frame_data& get_current_frame() { return m_frames[m_frame_number]; }

    private:
        GLFWwindow* m_glfwwindow;

        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::UniqueSurfaceKHR m_surface;
        vk_device m_device;
        vk_swapchain m_swapchain;

        VmaAllocator m_allocator{};

        std::array<frame_data, s_frames_in_flight> m_frames;
        uint32_t m_frame_number = 0;

        // immediate commands
        vk::UniqueCommandPool m_imm_command_pool;
        vk::UniqueCommandBuffer m_imm_command_buffer;
        vk::UniqueFence m_imm_fence;
    };
}
