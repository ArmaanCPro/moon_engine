#include "moonpch.h"

#include "vk_context.h"

#include <GLFW/glfw3.h>

#include "core/core.h"

namespace moon
{
    vk_context::vk_context(const native_handle& window)
    {
        MOON_CORE_ASSERT(window.type == NativeHandleType::GLFW, "Unsupported window type used with vulkan!");
        m_glfwwindow = std::get<GLFWwindow*>(window.handle);

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

        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].command_pool = m_device.create_command_pool(m_device.get_graphics_queue_index(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
            m_frames[i].command_buffer = m_device.allocate_command_buffer(m_frames[i].command_pool.get());
        }

        // sync structures
        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].render_fence = m_device.create_fence(true);

            m_frames[i].swapchain_semaphore = m_device.create_semaphore();
            m_frames[i].render_semaphore = m_device.create_semaphore();
        }

        // custom offscreen draw images
        vk::Extent3D extent = vk::Extent3D{m_swapchain.get_extent()};
        m_draw_image = m_device.allocate_image(extent, vk::Format::eR8G8B8A8Unorm,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

        m_depth_image = m_device.allocate_image(extent, vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment
        );
    }

    vk_context::~vk_context()
    {
        m_device.destroy_image(m_depth_image);
        m_device.destroy_image(m_draw_image);
        vkb::destroy_debug_utils_messenger(m_instance.get(), m_debug_messenger);
    }

    void vk_context::init()
    {
    }

    void vk_context::swap_buffers()
    {}

    void vk_context::begin_frame()
    {
        auto& frame = get_current_frame();

        m_device.wait_for_fence(frame.render_fence.get());
        m_device.reset_fence(frame.render_fence.get());

        auto [result, img_index] = m_swapchain.acquire_next_image(m_device.get_device(), frame.swapchain_semaphore.get());
        m_swapchain_image_index = img_index;
        if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
        {
            recreate_swapchain();
            return;
        }

        m_device.reset_command_pool(frame.command_pool.get());

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        frame.command_buffer->begin(beginInfo);
    }

    void vk_context::end_frame()
    {
        auto& frame = get_current_frame();
        frame.command_buffer->end();

        // after renderer_api is done, the swapchain format should be in ColorAttachmentOptimal
        transition_image(frame.command_buffer.get(), m_swapchain.get_images()[m_swapchain_image_index],
            vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

        auto result = m_device.submit_and_present(frame.command_buffer.get(), frame.swapchain_semaphore.get(),
            frame.render_semaphore.get(), frame.render_fence.get(), m_swapchain.get_swapchain(), m_swapchain_image_index);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            recreate_swapchain();
        }

        m_frame_number = (m_frame_number + 1) % s_frames_in_flight;
    }

    void vk_context::recreate_swapchain()
    {
        m_device.get_device().waitIdle();

        int width, height;
        glfwGetFramebufferSize(m_glfwwindow, &width, &height);

        auto newSwap = vk_swapchain{ m_glfwwindow, m_surface.get(), m_instance.get(),
                                   m_device.get_physical_device(), m_device.get_device(), vk::PresentModeKHR::eFifo,
        std::move(m_swapchain)};
        m_swapchain = std::move(newSwap);
        m_resize_requested = false;
    }
}
