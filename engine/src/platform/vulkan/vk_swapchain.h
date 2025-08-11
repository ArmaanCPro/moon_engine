#pragma once

#include "vk.h"

struct GLFWwindow;
namespace moon
{
    class vk_swapchain
    {
    public:
        vk_swapchain() = default;
        vk_swapchain(GLFWwindow* window, vk::SurfaceKHR surface, vk::Instance instance, vk::PhysicalDevice physical_device,
            vk::Device device, vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo,
            std::optional<vk_swapchain> old_swapchain = {},
            vk::Format format = vk::Format::eB8G8R8Unorm);

        // returns vk::Result and next image index
        vk::ResultValue<uint32_t> acquire_next_image(vk::Device device, vk::Semaphore wait_semaphore);

        vk::Extent2D get_extent() const { return m_swapchain_extent; }
        vk::Format get_format() const { return m_swapchain_image_format; }
        const std::vector<vk::ImageView>& get_image_views() const { return m_swapchain_image_views; }
        vk::SwapchainKHR get_swapchain() const { return m_swapchain.get(); }
    private:
        vk::UniqueSwapchainKHR m_swapchain;

        vk::Format m_swapchain_image_format;
        vk::Extent2D m_swapchain_extent;
        std::vector<vk::Image> m_swapchain_images;
        std::vector<vk::ImageView> m_swapchain_image_views;
    };
}
