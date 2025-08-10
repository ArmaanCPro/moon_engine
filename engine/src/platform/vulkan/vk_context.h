#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"

namespace moon
{
    struct QueueFamilies
    {
        vk::Queue graphics_queue;
        uint32_t graphics_queue_index;
    };

    class vk_context final : public graphics_context
    {
    public:
        vk_context(const native_handle& window);
        ~vk_context();

        void init() override;
        void swap_buffers() override;

    private:
        GLFWwindow* m_glfwwindow;

        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::UniqueSurfaceKHR m_surface;
        vk::PhysicalDevice m_physical_device;
        vk::UniqueDevice m_device;

        QueueFamilies m_queue_families;

        VmaAllocator m_allocator{};
    };
}
