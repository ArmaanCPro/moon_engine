#include "moonpch.h"

#include "vk_context.h"

#include <GLFW/glfw3.h>

#include "core/core.h"

namespace moon
{
    vk_context::vk_context(const native_handle& window)
    {
        MOON_CORE_ASSERT(window.type == NativeHandleType::GLFW, "Unsupported window type used with vulkan!");
        init();
    }

    vk_context::~vk_context()
    {
        vmaDestroyAllocator(m_allocator);
    }

    void vk_context::init()
    {
        //uint32_t glfwExtensionCount = 0;
        //const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vkb::InstanceBuilder instanceBuilder;
        auto instRet = instanceBuilder
            .set_app_name("Moon Engine")
            .request_validation_layers(true)
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger()
            .build();

        vkb::Instance vkbInstance = instRet.value();

        m_instance = vk::UniqueInstance{vkbInstance.instance};
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance.get(), reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress));
        m_debug_messenger = vkbInstance.debug_messenger;

        glfwCreateWindowSurface(m_instance.get(), m_glfwwindow, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface.get()));

        // vk 1.3 features
        vk::PhysicalDeviceVulkan13Features features;
        features.dynamicRendering = true;
        features.synchronization2 = true;
        // vk 1.2 features
        vk::PhysicalDeviceVulkan12Features features12;
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        vkb::PhysicalDeviceSelector selector{vkbInstance};
        vkb::PhysicalDevice vkbPhysicalDevice = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(m_surface.get())
            .select()
            .value();

        vkb::DeviceBuilder deviceBuilder{vkbPhysicalDevice};
        vkb::Device vkbDevice = deviceBuilder.build().value();

        m_physical_device = vkbPhysicalDevice.physical_device;
        m_device = vk::UniqueDevice{vkbDevice.device};

        m_queue_families.graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        m_queue_families.graphics_queue_index = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        VmaAllocatorCreateInfo allocatorCI{};
        allocatorCI.physicalDevice = m_physical_device;
        allocatorCI.device = m_device.get();
        allocatorCI.instance = m_instance.get();
        allocatorCI.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorCI, &m_allocator);
    }

    void vk_context::swap_buffers()
    {}
}
