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

        // command buffers
        vk::CommandPoolCreateInfo command_poolCI{};
        command_poolCI.queueFamilyIndex = m_device.get_graphics_queue_index();
        command_poolCI.flags = vk::CommandPoolCreateFlagBits::eTransient;

        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].command_pool = m_device.get_device().createCommandPoolUnique(command_poolCI);

            vk::CommandBufferAllocateInfo cmdAllocInfo{};
            cmdAllocInfo.commandPool = m_frames[i].command_pool.get();
            cmdAllocInfo.commandBufferCount = 1;

            m_frames[i].command_buffer = std::move(m_device.get_device().allocateCommandBuffersUnique(cmdAllocInfo).front());
        }
        m_imm_command_pool = m_device.get_device().createCommandPoolUnique(command_poolCI);
        vk::CommandBufferAllocateInfo cmdAllocInfo{};
        cmdAllocInfo.commandPool = m_imm_command_pool.get();
        cmdAllocInfo.commandBufferCount = 1;
        m_imm_command_buffer = std::move(m_device.get_device().allocateCommandBuffersUnique(cmdAllocInfo).front());

        // sync structures
        vk::FenceCreateInfo fenceCI{};
        fenceCI.flags = vk::FenceCreateFlagBits::eSignaled; // start fence signaled so we can wait on it first frame
        vk::SemaphoreCreateInfo semaphoreCI{};
        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].render_fence = m_device.get_device().createFenceUnique(fenceCI);

            m_frames[i].swapchain_semaphore = m_device.get_device().createSemaphoreUnique(semaphoreCI);
            m_frames[i].render_semaphore = m_device.get_device().createSemaphoreUnique(semaphoreCI);
        }
        m_imm_fence = m_device.get_device().createFenceUnique(fenceCI);
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

    void vk_context::begin_frame()
    {
        [[maybe_unused]] auto result = m_device.get_device().waitForFences(1, &get_current_frame().render_fence.get(), vk::True, std::numeric_limits<uint64_t>::max());
    }

    void vk_context::end_frame()
    {
        m_frame_number = (m_frame_number + 1) % s_frames_in_flight;
    }
}
