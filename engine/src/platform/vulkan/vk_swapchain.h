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
    private:
        vk::UniqueSwapchainKHR m_swapchain;

        vk::Format m_swapchain_image_format;
        vk::Extent2D m_swapchain_extent;
        std::vector<vk::Image> m_swapchain_images;
        std::vector<vk::ImageView> m_swapchain_image_views;
    };
}
