#pragma once

#include "vk.h"
#include "renderer/handle.h"

struct GLFWwindow;
namespace moon::vulkan
{
    class vk_context;

    class vk_swapchain final
    {
    public:
        static constexpr auto s_max_swapchain_images = 16;

        vk_swapchain(vk_context& context, GLFWwindow* window, vk::SurfaceKHR surface, vk::Instance instance, vk::PhysicalDevice physical_device,
            vk::Device device, vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo,
            std::optional<vk_swapchain> old_swapchain = {},
            vk::Format format = vk::Format::eB8G8R8Unorm);
        ~vk_swapchain();

        // returns vk::Result and next image index
        vk::ResultValue<uint32_t> acquire_next_image(vk::Device device, vk::Semaphore wait_semaphore);

        vk::Result present(vk::Semaphore wait_semaphore);
        [[nodiscard]] vk::Image get_current_image() const;
        [[nodiscard]] vk::ImageView get_current_image_view() const;
        [[nodiscard]] texture_handle get_current_texture();
        [[nodiscard]] const vk::SurfaceFormatKHR& get_surface_format() const { return m_surface_format; }
        [[nodiscard]] uint32_t get_swapchain_image_count() const { return m_swapchain_image_count; }
        [[nodiscard]] uint32_t get_current_frame_index() const { return m_current_frame; }

        [[nodiscard]] vk::Extent2D get_extent() const { return m_swapchain_extent; }
        [[nodiscard]] vk::Format get_format() const { return m_swapchain_image_format; }
        [[nodiscard]] vk::SwapchainKHR get_swapchain() const { return m_swapchain.get(); }

        void set_timeline_wait_value(uint32_t image_index, uint64_t value) { m_timeline_wait_values[image_index] = value; }
        [[nodiscard]] uint64_t get_current_timeline_wait_value(uint32_t image_index) const { return m_timeline_wait_values[image_index]; }
    private:
        vk::UniqueSwapchainKHR m_swapchain;
        vk::Device m_device;
        vk::Queue m_graphics_queue;
        vk_context& m_context;

        uint32_t m_swapchain_image_count = 0;
        uint32_t m_current_image = 0; // [0, swapchain_image_count]
        uint32_t m_current_frame = 0; // [0, +inf]
        bool m_get_next_image = true;

        vk::SurfaceFormatKHR m_surface_format = vk::Format::eUndefined;
        vk::Format m_swapchain_image_format;
        vk::Extent2D m_swapchain_extent;
        std::array<texture_handle, s_max_swapchain_images> m_swapchain_textures;
        std::array<vk::UniqueSemaphore, s_max_swapchain_images> m_acquire_semaphores;
        std::array<vk::UniqueFence, s_max_swapchain_images> m_present_fences;
        std::array<uint64_t, s_max_swapchain_images> m_timeline_wait_values;
    };
}
