#include "moonpch.h"
#include "vk_device.h"

#include <GLFW/glfw3.h>

namespace moon::vulkan
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
        m_queue_families.compute_queue = vkb_device.get_queue(vkb::QueueType::compute).value();
        m_queue_families.compute_queue_index = vkb_device.get_queue_index(vkb::QueueType::compute).value();

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

    void vk_device::record(vk::CommandBuffer cmd)
    {

    }

    vk::UniqueSemaphore vk_device::create_semaphore() const
    {
        vk::SemaphoreCreateInfo ci{};
        return m_device->createSemaphoreUnique(ci);
    }

    vk::UniqueFence vk_device::create_fence(bool start_open) const
    {
        vk::FenceCreateInfo ci{};
        if (start_open)
        {
            ci.flags |= vk::FenceCreateFlagBits::eSignaled;
        }
        return m_device->createFenceUnique(ci);
    }

    void vk_device::wait_for_fence(vk::Fence fence)
    {
        while (m_device->waitForFences(1, &fence, vk::True, std::numeric_limits<uint64_t>::max()) == vk::Result::eTimeout)
        {}
    }

    void vk_device::reset_fence(vk::Fence fence)
    {
        while (m_device->resetFences(1, &fence) == vk::Result::eTimeout)
        {}
    }

    vk::UniqueCommandPool vk_device::create_command_pool(uint32_t queue_index, vk::CommandPoolCreateFlagBits flags) const
    {
        vk::CommandPoolCreateInfo ci{};
        ci.queueFamilyIndex = queue_index;
        ci.flags = flags;
        return m_device->createCommandPoolUnique(ci);
    }

    vk::UniqueCommandBuffer vk_device::allocate_command_buffer(vk::CommandPool pool, vk::CommandBufferLevel level) const
    {
        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.commandPool = pool;
        alloc_info.commandBufferCount = 1;
        alloc_info.level = level;
        return std::move(m_device->allocateCommandBuffersUnique(alloc_info).front());
    }

    void vk_device::reset_command_pool(vk::CommandPool pool)
    {
        m_device->resetCommandPool(pool);
    }

    void vk_device::immediate_submit(std::function<void(vk::CommandBuffer)>&& fn)
    {
        auto pool = create_command_pool(m_queue_families.compute_queue_index,
            vk::CommandPoolCreateFlagBits::eTransient);
        auto cmd = allocate_command_buffer(pool.get());

        cmd->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        fn(cmd.get());
        cmd->end();

        auto fence = create_fence(false);
        vk::CommandBufferSubmitInfo cmdSI{};
        cmdSI.commandBuffer = cmd.get();
        vk::SubmitInfo2 submit_info{};
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cmdSI;
        [[maybe_unused]] auto result = m_queue_families.compute_queue.submit2(1, &submit_info, fence.get());
        wait_for_fence(fence.get());
    }

    vk::Result vk_device::submit(vk::Queue queue, vk::CommandBuffer cmd, vk::Semaphore wait_semaphore,
                                 vk::PipelineStageFlags2 wait_stage, vk::Semaphore signal_semaphore,
                                 vk::Fence signal_fence)
    {
        vk::CommandBufferSubmitInfo cmdSI{};
        cmdSI.commandBuffer = cmd;
        vk::SemaphoreSubmitInfo waitInfo{};
        waitInfo.semaphore = wait_semaphore;
        waitInfo.value = 1;
        waitInfo.stageMask = wait_stage;
        vk::SemaphoreSubmitInfo signalInfo{};
        signalInfo.semaphore = signal_semaphore;
        signalInfo.value = 1;
        signalInfo.stageMask = vk::PipelineStageFlagBits2::eAllGraphics;
        vk::SubmitInfo2 submit_info{};
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cmdSI;
        submit_info.waitSemaphoreInfoCount = 1;
        submit_info.pWaitSemaphoreInfos = &waitInfo;
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &signalInfo;

        return queue.submit2(1, &submit_info, signal_fence);
    }

    vk::Result vk_device::submit_and_present(vk::CommandBuffer cmd, vk::Semaphore wait_semaphore, vk::Semaphore signal_semaphore,
        vk::Fence signal_fence, vk::SwapchainKHR swapchain, uint32_t image_index)
    {
        submit(m_queue_families.graphics_queue, cmd, wait_semaphore, vk::PipelineStageFlagBits2::eColorAttachmentOutput, signal_semaphore, signal_fence);

        vk::PresentInfoKHR present_info{};
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &signal_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr;

        return m_queue_families.graphics_queue.presentKHR(present_info);
    }

    allocated_image vk_device::allocate_image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped) const
    {
        allocated_image return_image;
        return_image.extent = extent;
        return_image.format = format;

        vk::ImageCreateInfo imgCI{};
        imgCI.imageType = extent.depth == 1 ? vk::ImageType::e2D : vk::ImageType::e3D;
        imgCI.format = format;
        imgCI.mipLevels = 1;
        if (mipmapped)
        {
            imgCI.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
        }
        imgCI.arrayLayers = 1;
        imgCI.samples = vk::SampleCountFlagBits::e1;
        imgCI.usage = usage;

        // always allocate images on gpu
        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        alloc_info.requiredFlags = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(m_allocator, reinterpret_cast<VkImageCreateInfo*>(&imgCI), &alloc_info,
            reinterpret_cast<VkImage*>(&return_image.image), &return_image.allocation, nullptr);

        vk::ImageAspectFlags aspect_flag = vk::ImageAspectFlagBits::eColor;
        if (format == vk::Format::eD32Sfloat)
        {
            aspect_flag = vk::ImageAspectFlagBits::eDepth;
        }

        vk::ImageViewCreateInfo img_viewCI{};
        img_viewCI.format = format;
        img_viewCI.image = return_image.image;
        img_viewCI.viewType = extent.depth == 1 ? vk::ImageViewType::e2D : vk::ImageViewType::e3D;
        img_viewCI.subresourceRange.aspectMask = aspect_flag;
        img_viewCI.subresourceRange.levelCount = imgCI.mipLevels;
        img_viewCI.subresourceRange.baseMipLevel = 0;
        img_viewCI.subresourceRange.levelCount = 1;
        img_viewCI.subresourceRange.baseArrayLayer = 0;
        img_viewCI.subresourceRange.layerCount = 1;

        return_image.view = m_device->createImageView(img_viewCI);

        return return_image;
    }

    void vk_device::destroy_image(allocated_image& image) const
    {
        if (image.image)
        {
            m_device->destroyImageView(image.view);
            vmaDestroyImage(m_allocator, image.image, image.allocation);
        }
        image.image = VK_NULL_HANDLE;
        image.allocation = VK_NULL_HANDLE;
    }

    VkDeviceAddress vk_device::get_buffer_device_address(vk::Buffer buffer) const
    {
        vk::BufferDeviceAddressInfo info{};
        info.buffer = buffer;
        return m_device->getBufferAddress(info);
    }
}
