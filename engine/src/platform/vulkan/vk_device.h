#pragma once

#include "renderer/device.h"

#include "vk.h"

namespace moon
{
    struct queue_families
    {
        vk::Queue graphics_queue;
        uint32_t graphics_queue_index;
    };

    class vk_device final : public device
    {
    public:
        vk_device() = default;
        void init(vkb::Instance vkb_instance, vk::SurfaceKHR surface);
        ~vk_device() override;

        vk::PhysicalDevice get_physical_device() const { return m_physical_device; }
        vk::Device get_device() const { return m_device.get(); }

        vk::Queue get_graphics_queue() const { return m_queue_families.graphics_queue; }

        VmaAllocator get_allocator() const { return m_allocator; }
    private:
        vk::PhysicalDevice m_physical_device;
        vk::UniqueDevice m_device;

        queue_families m_queue_families;

        VmaAllocator m_allocator = VK_NULL_HANDLE;

        bool m_initialized = false;
    };
}
