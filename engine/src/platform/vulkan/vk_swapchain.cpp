#include "moonpch.h"
#include "vk_swapchain.h"

#include "core/window.h"

#include <GLFW/glfw3.h>

namespace moon
{
    vk_swapchain::vk_swapchain(GLFWwindow* window, vk::SurfaceKHR surface, vk::Instance instance,
        vk::PhysicalDevice physical_device, vk::Device device, vk::PresentModeKHR present_mode,
        std::optional<vk_swapchain> old_swapchain, vk::Format format)
    {
        glfwCreateWindowSurface(instance, window, nullptr, (VkSurfaceKHR*)&surface);

        vkb::SwapchainBuilder swapchain_builder{physical_device, device, surface};

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        m_swapchain_image_format = format;

        vkb::Swapchain vkbSwapchain = swapchain_builder
            .set_desired_format(vk::SurfaceFormatKHR(m_swapchain_image_format,
                        vk::ColorSpaceKHR::eSrgbNonlinear))
            .set_desired_present_mode(static_cast<VkPresentModeKHR>(present_mode))
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .set_old_swapchain(old_swapchain.has_value() ? old_swapchain.value().m_swapchain.get() : VK_NULL_HANDLE)
            .build()
            .value();

        m_swapchain_extent = vkbSwapchain.extent;
        m_swapchain = vk::UniqueSwapchainKHR{vkbSwapchain.swapchain};

        for (auto img : vkbSwapchain.get_images().value())
        {
            m_swapchain_images.emplace_back(img);
        }
        for (auto img_view : vkbSwapchain.get_image_views().value())
        {
            m_swapchain_image_views.emplace_back(img_view);
        }
    }

    vk::ResultValue<uint32_t> vk_swapchain::acquire_next_image(vk::Device device, vk::Semaphore wait_semaphore)
    {
        return device.acquireNextImageKHR(m_swapchain.get(), std::numeric_limits<uint64_t>::max(), wait_semaphore, nullptr);
    }
}
