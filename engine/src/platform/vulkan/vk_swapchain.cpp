#include "moonpch.h"
#include "vk_swapchain.h"

#include "core/window.h"

#include <GLFW/glfw3.h>
#include "vk_context.h"
#include "utils/vk_utils.h"

namespace moon::vulkan
{
    vk_swapchain::vk_swapchain(vk_context& context, GLFWwindow* window, vk::SurfaceKHR surface, vk::Instance instance,
        vk::PhysicalDevice physical_device, vk::Device device, vk::PresentModeKHR present_mode,
        std::optional<vk_swapchain> old_swapchain, vk::Format format)
        :
        m_device(device),
        m_context(context),
        m_timeline_wait_values()
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

        m_swapchain_image_count = vkbSwapchain.image_count;
        MOON_CORE_ASSERT(m_swapchain_image_count <= s_max_swapchain_images);
        m_swapchain_image_count = s_max_swapchain_images;

        const auto usage_flags = [&]() -> vk::ImageUsageFlags
        {
            const auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);
            const auto props = physical_device.getFormatProperties(format);

            bool storage_usage_supported = (caps.supportedUsageFlags & vk::ImageUsageFlagBits::eStorage) !=
                                           vk::ImageUsageFlags{}
                                           && (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage)
                                           != vk::FormatFeatureFlags{};

            vk::ImageUsageFlags usages = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst
                                         | vk::ImageUsageFlagBits::eTransferSrc;

            if (storage_usage_supported)
                usages |= vk::ImageUsageFlagBits::eStorage;

            return usages;
        }();

        char debug_name_image[256] = {};
        char debug_name_view[256] = {};
        for (auto i = 0u; i < m_swapchain_image_count; ++i)
        {
            m_acquire_semaphores[i] = vk::UniqueSemaphore{
                utils::create_semaphore(m_device, "Semaphore: swapchain-acquire")};

            std::snprintf(debug_name_image, sizeof(debug_name_image) - 1, "Image: swapchain %u", i);
            std::snprintf(debug_name_view, sizeof(debug_name_view) - 1, "Image View: swapchain %u", i);
            vulkan_image image = {
                .m_image = vkbSwapchain.get_images().value()[i],
                .m_usage_flags = usage_flags,
                .m_format = m_swapchain_image_format,
                .m_extent = vk::Extent3D{m_swapchain_extent.width, m_swapchain_extent.height, 1},
                .m_type = vk::ImageType::e2D,
                .m_is_swapchain_image = true,
                .m_is_owning_vk_image = false,
                .m_is_depth_format = utils::is_depth_format(m_swapchain_image_format),
                .m_is_stencil_format = utils::is_stencil_format(m_swapchain_image_format)
            };

            VK_CHECK(
                utils::set_debug_object_name(m_device, vk::ObjectType::eImage, std::bit_cast<uint64_t>(image.m_image),
                    debug_name_image));

            image.m_image_view = image.create_image_view(m_device, vk::ImageViewType::e2D, image.m_format,
                                                         vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1, {}, nullptr,
                                                         debug_name_view);

            m_swapchain_textures[i] = m_context.m_textures_pool.create(std::move(image));
        }
    }

    vk_swapchain::~vk_swapchain()
    {
        for (auto image : m_swapchain_textures)
        {
            m_context.m_textures_pool.destroy(image);
        }
    }

    vk::ResultValue<uint32_t> vk_swapchain::acquire_next_image(vk::Device device, vk::Semaphore wait_semaphore)
    {
        return device.acquireNextImageKHR(m_swapchain.get(), std::numeric_limits<uint64_t>::max(), wait_semaphore, nullptr);
    }

    vk::Result vk_swapchain::present(vk::Semaphore wait_semaphore)
    {
        const vk::SwapchainPresentFenceInfoEXT fence_info = {
            1, &m_present_fences[m_current_image].get()
        };
        const vk::PresentInfoKHR present_info = {
            1, &m_acquire_semaphores[m_current_image].get(),
            1u,
            &m_swapchain.get(),
            &m_current_image,
            nullptr, &fence_info
        };
        if (m_context.get_device().has_EXT_swapchain_maintenance1())
        {
            if (!m_present_fences[m_current_image])
                m_present_fences[m_current_image] = vk::UniqueFence{ utils::create_fence(m_device, "Fence: present-fence") };
        }

        auto r = m_graphics_queue.presentKHR(&present_info);
        if (r != vk::Result::eSuboptimalKHR && r != vk::Result::eErrorOutOfDateKHR && r != vk::Result::eSuccess)
        {
            MOON_CORE_ASSERT_MSG(false, "Failed to present swapchain image {0}", vk::to_string(r));
        }

        // ready to call acquire_next_image() on the next get_current_vulkan_texture()
        m_get_next_image = true;
        m_current_frame++;

        return r;
    }

    vk::Image vk_swapchain::get_current_image() const
    {
        return m_context.m_textures_pool.get(m_swapchain_textures[m_current_image])->m_image;
    }

    vk::ImageView vk_swapchain::get_current_image_view() const
    {
        return m_context.m_textures_pool.get(m_swapchain_textures[m_current_image])->m_image_view;
    }

    texture_handle vk_swapchain::get_current_texture()
    {
        if (m_get_next_image)
        {
            if (m_present_fences[m_current_image])
            {
                // VK_KHR_swapchain_maintenance1: before acquiring again, wait for the presentation operation to finish
                VK_CHECK(m_device.waitForFences(1, &m_present_fences[m_current_image].get(), vk::True, std::numeric_limits<uint64_t>::max()));
                VK_CHECK(m_device.resetFences(1, &m_present_fences[m_current_image].get()));
            }

            const vk::SemaphoreWaitInfo wait_info = {
                {}, 1, &m_context.m_timeline_semaphore.get(),
                &m_timeline_wait_values[m_current_image]
            };
            VK_CHECK(m_device.waitSemaphores(wait_info, std::numeric_limits<uint64_t>::max()));

            vk::Semaphore acquire_semaphore = m_acquire_semaphores[m_current_image].get();
            auto r = m_device.acquireNextImageKHR(m_swapchain.get(), std::numeric_limits<uint64_t>::max(), acquire_semaphore, VK_NULL_HANDLE);
            m_current_image = r.value;

            if (r.result != vk::Result::eSuccess && r.result != vk::Result::eSuboptimalKHR && r.result != vk::Result::eErrorOutOfDateKHR)
            {
                MOON_CORE_ASSERT_MSG(false, "Failed to acquire next swapchain image");
            }

            m_get_next_image = false;
            m_context.m_immediate->wait_semaphore(acquire_semaphore);
        }

        return m_swapchain_textures[m_current_image];
    }
}
