#include "moonpch.h"
#include "vk_device.h"

#include <GLFW/glfw3.h>

namespace moon
{
    void vk_device::init(vkb::Instance vkb_instance, vk::SurfaceKHR surface)
    {
        MOON_CORE_ASSERT(!m_initialized, "Called init on device when it is already initialized!");

        // vk 1.3 features
        vk::PhysicalDeviceVulkan13Features features;
        features.dynamicRendering = true;
        features.synchronization2 = true;
        // vk 1.2 features
        vk::PhysicalDeviceVulkan12Features features12;
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        // extension features for descriptor buffers
        vk::PhysicalDeviceDescriptorBufferFeaturesEXT desc_buffer_features{};
        desc_buffer_features.descriptorBuffer = VK_TRUE;

        vkb::PhysicalDeviceSelector selector{vkb_instance};
        vkb::PhysicalDevice vkb_physical_device = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .add_required_extension(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME)
            .add_required_extension_features(desc_buffer_features)
            .set_surface(surface)
            .select()
            .value();

        vkb::DeviceBuilder device_builder{vkb_physical_device};
        vkb::Device vkb_device = device_builder.build().value();

        m_physical_device = vkb_physical_device.physical_device;
        m_device = vk::UniqueDevice{vkb_device.device};

        m_queue_families.graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
        m_queue_families.graphics_queue_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

        VmaAllocatorCreateInfo allocatorCI{};
        allocatorCI.physicalDevice = m_physical_device;
        allocatorCI.device = m_device.get();
        allocatorCI.instance = vkb_instance.instance;
        allocatorCI.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorCI, &m_allocator);

        m_initialized = true;
    }

    vk_device::~vk_device()
    {
        if (m_allocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }
    }
}
