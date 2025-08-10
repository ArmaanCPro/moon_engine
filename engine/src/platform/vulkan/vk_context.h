#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"
#include "vk_device.h"

#include "vk_swapchain.h"

namespace moon
{
    class vk_context final : public graphics_context
    {
    public:
        vk_context(const native_handle& window);
        ~vk_context();

        void init() override;
        void swap_buffers() override;

        device& get_device() override { return m_device; }

    private:
        GLFWwindow* m_glfwwindow;

        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::UniqueSurfaceKHR m_surface;
        vk_device m_device;
        vk_swapchain m_swapchain;

        VmaAllocator m_allocator{};
    };
}
