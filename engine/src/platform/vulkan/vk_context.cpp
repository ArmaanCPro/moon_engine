#include "moonpch.h"

#include "vk_context.h"

#include <GLFW/glfw3.h>

#include "core/core.h"

namespace moon
{
    vk_context::vk_context(const native_handle& window)
    {
        MOON_CORE_ASSERT(window.type == NativeHandleType::GLFW, "Unsupported window type used with vulkan!");
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vkb::InstanceBuilder instanceBuilder;
        auto instRet = instanceBuilder
            .set_app_name("Moon Engine")
            .request_validation_layers(true)
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger()
            .enable_extensions(glfwExtensionCount, glfwExtensions)
            .build();

        vkb::Instance vkbInstance = instRet.value();

        m_instance = vk::UniqueInstance{vkbInstance.instance};
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance.get(),
                                           reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress));
        m_debug_messenger = vkbInstance.debug_messenger;

        glfwCreateWindowSurface(m_instance.get(), m_glfwwindow, nullptr,
                                reinterpret_cast<VkSurfaceKHR*>(&m_surface.get()));

        m_device.init(vkbInstance, m_surface.get());
        m_swapchain = vk_swapchain{ m_glfwwindow, m_surface.get(), m_instance.get(),
                                   m_device.get_physical_device(), m_device.get_device() };
    }

    vk_context::~vk_context()
    {
        vmaDestroyAllocator(m_allocator);
    }

    void vk_context::init()
    {

    }

    void vk_context::swap_buffers()
    {}
}
