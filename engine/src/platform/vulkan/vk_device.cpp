#include "moonpch.h"
#include "vk_device.h"

#include "utils/vk_utils.h"

#include <set>
#include <GLFW/glfw3.h>

#include "vk_context.h"

namespace moon::vulkan
{
    void vk_device::init(vkb::Instance vkb_instance, vk::SurfaceKHR surface, vk_context* context)
    {
        m_context = context;

        MOON_CORE_ASSERT_MSG(!m_initialized, "Called init on device when it is already initialized!");

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

        m_physical_device_properties = m_physical_device.getProperties2().properties;

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

        static constexpr std::array depth_formats = {
            vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD16Unorm
        };
        for (vk::Format depth_format : depth_formats) {
            VkFormatProperties format_props = m_physical_device.getFormatProperties(depth_format);

            if (format_props.optimalTilingFeatures) {
                m_supported_depth_formats.push_back(depth_format);
            }
        }

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

    std::expected<vulkan_image, result> vk_device::allocate_texture(texture_desc desc, const char* debug_name)
    {
        if (debug_name && *debug_name)
            desc.debug_name = debug_name;

        const vk::Format vk_format = is_depth_or_stencil_format(desc.format) ? get_closest_depth_stencil_format(desc.format)
            : utils::format_to_vk_format(desc.format);

        MOON_CORE_ASSERT(vk_format != vk::Format::eUndefined);

        const TextureType type = desc.type;
        if (type != TextureType::e2D && type != TextureType::e3D && type != TextureType::eCube)
        {
            MOON_CORE_ERROR("Only 2D, 3D, and Cube textures are supported!");
            return std::unexpected(result{result::Code::RuntimeError});
        }
        if (desc.num_mip_levels == 0)
        {
            MOON_CORE_ERROR("Texture must have at least 1 mip level!");
            desc.num_mip_levels = 1;
        }
        if (desc.num_samples > 1 && desc.num_mip_levels != 1)
        {
            MOON_CORE_ERROR("Texture must have 1 mip level when multisampling is enabled!");
            return std::unexpected(result{result::Code::ArgumentOutOfRange});
        }
        if (desc.num_samples > 1 && type == TextureType::e3D)
        {
            MOON_CORE_ERROR("3D textures cannot be multisampled!");
            return std::unexpected(result{result::Code::ArgumentOutOfRange});
        }
        if (desc.usage == 0)
        {
            MOON_CORE_ERROR("Texture must have at least 1 usage!");
            return std::unexpected(result{result::Code::ArgumentOutOfRange});
        }

        // use staging device to transfer data into the image if storage is device local
        const vk::ImageUsageFlags usage_flags = [usages = desc.usage, storage_type = desc.storage_type, format = desc.format]()
        {
            auto flags = (storage_type == StorageType::Device) ? vk::ImageUsageFlagBits::eTransferDst : vk::ImageUsageFlags{};
            if (usages & static_cast<uint8_t>(TextureUsageBits::Sampled))
                flags |= vk::ImageUsageFlagBits::eSampled;
            if (usages & static_cast<uint8_t>(TextureUsageBits::Storage))
                flags |= vk::ImageUsageFlagBits::eStorage;
            if (usages & static_cast<uint8_t>(TextureUsageBits::Attachment))
            {
                flags |= is_depth_or_stencil_format(format) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment;
                if (storage_type == StorageType::MemoryLess)
                    flags |= vk::ImageUsageFlagBits::eTransientAttachment;
            }

            if (storage_type != StorageType::MemoryLess)
            {
                // for now, always set this flag so we can read it back
                flags |= vk::ImageUsageFlagBits::eTransferSrc;
            }
            return flags;
        }();

        MOON_CORE_ASSERT_MSG(usage_flags, "invalid texture usage!");

        const vk::MemoryPropertyFlags memory_flags = utils::storage_type_to_vk_memory_property_flags(desc.storage_type);

        const bool has_debug_name = desc.debug_name && *desc.debug_name;

        char debug_name_image[256] = {};
        char debug_name_view[256] = {};

        if (has_debug_name)
        {
            std::snprintf(debug_name_image, 256, "Image: %s", desc.debug_name);
            std::snprintf(debug_name_view, 256, "View: %s", desc.debug_name);
        }

        vk::ImageCreateFlags vk_create_flags = {};
        vk::ImageViewType vk_view_type;
        vk::ImageType vk_image_type;
        vk::SampleCountFlagBits vk_samples = vk::SampleCountFlagBits::e1;
        uint32_t num_layers = desc.num_layers;

        switch (desc.type)
        {
        case TextureType::e2D:
            vk_view_type = num_layers > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
            vk_image_type = vk::ImageType::e2D;
            vk_samples = utils::get_vulkan_samples_count_flags(desc.num_samples, vk::SampleCountFlagBits{get_framebuffer_msaa_bitmask()});
            break;
        case TextureType::e3D:
            vk_view_type = vk::ImageViewType::e3D;
            vk_image_type = vk::ImageType::e3D;
            break;
        case TextureType::eCube:
            vk_view_type = num_layers > 1 ? vk::ImageViewType::eCubeArray : vk::ImageViewType::eCube;
            vk_image_type = vk::ImageType::e2D;
            vk_create_flags = vk::ImageCreateFlagBits::eCubeCompatible;
            num_layers *= 6;
            break;
        default:
            MOON_CORE_ERROR("Invalid texture type!");
            return std::unexpected(result{result::Code::RuntimeError});
        }

        const vk::Extent3D vk_extent {desc.dimensions.width, desc.dimensions.height, desc.dimensions.depth};
        const uint32_t num_levels = desc.num_mip_levels;

        MOON_CORE_ASSERT_MSG(num_levels > 0, "Image must contain at least 1 mip level");
        MOON_CORE_ASSERT_MSG(num_layers > 0, "Image must contain at least 1 layer");
        MOON_CORE_ASSERT_MSG(static_cast<uint8_t>(vk_samples) != 0, "Image must have at least 1 sample");

        vulkan_image image = {
            .m_usage_flags = usage_flags,
            .m_format = vk_format,
            .m_extent = vk_extent,
            .m_type = vk_image_type,
            .m_samples = vk_samples,
            .m_num_levels = num_levels,
            .m_num_layers = num_layers,
            .m_is_depth_format = utils::is_depth_format(vk_format),
            .m_is_stencil_format = utils::is_stencil_format(vk_format),
        };

        if (has_debug_name)
            std::snprintf(image.m_debug_name, sizeof(image.m_debug_name) - 1, "%s", desc.debug_name);


        // TODO num_planes and is_disjoint

        const vk::ImageCreateInfo ci = {
            vk_create_flags, vk_image_type, vk_format, vk_extent, num_levels, num_layers, vk_samples,
            vk::ImageTiling::eOptimal, usage_flags, vk::SharingMode::eExclusive
        };

        VmaAllocationCreateInfo alloc_info{
            .usage = memory_flags & vk::MemoryPropertyFlagBits::eHostVisible ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_AUTO
        };

        auto r = vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&ci), &alloc_info,
            reinterpret_cast<VkImage*>(&image.m_image), &image.m_allocation, nullptr);
        if (r != VK_SUCCESS)
        {
            MOON_CORE_ERROR("Failed to create image!");
            return std::unexpected(result{result::Code::RuntimeError, "vmaCreateImage() failed"});
        }

        // handle memory-mapped buffers
        if (memory_flags & vk::MemoryPropertyFlagBits::eHostVisible)
            vmaMapMemory(m_allocator, image.m_allocation, &image.m_mapped_ptr);

        VK_CHECK(utils::set_debug_object_name(m_device.get(), image.m_image.objectType, std::bit_cast<uint64_t>(image.m_image), debug_name_image));

        // get physical device's properties for the image's format
        image.m_format_properties = m_physical_device.getFormatProperties(image.m_format);

        vk::ImageAspectFlags aspect = {};
        if (image.m_is_depth_format)
            aspect = vk::ImageAspectFlagBits::eDepth;
        else if (image.m_is_stencil_format)
            aspect = vk::ImageAspectFlagBits::eStencil;
        else
            aspect = vk::ImageAspectFlagBits::eColor;

        const vk::ComponentMapping mapping = {
            static_cast<vk::ComponentSwizzle>(desc.swizzle.r),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.g),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.b),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.a)
        };

        // TODO ycbcr info

        image.m_image_view = image.create_image_view(m_device.get(), vk_view_type, vk_format, aspect, 0, vk::RemainingMipLevels, 0, num_layers,
            mapping, {}, debug_name_view);

        if (image.m_usage_flags & vk::ImageUsageFlagBits::eStorage)
        {
            if (!desc.swizzle.identity())
            {
                // use identity swizzle for storage images
                image.m_image_view_storage = image.create_image_view(m_device.get(), vk_view_type, vk_format, aspect, 0, vk::RemainingMipLevels, 0, num_layers,
                    {}, {}, debug_name_view);
            }
        }

        if (image.m_image_view == VK_NULL_HANDLE)
        {
            MOON_CORE_ERROR("Failed to create image view!");
            return std::unexpected(result{result::Code::RuntimeError, "Failed to create image view"});
        }

        return image;
    }

    vulkan_buffer vk_device::allocate_buffer(vk::DeviceSize buffer_size,
                                             vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags mem_flags,
                                             const char* debug_name)
    {
        MOON_CORE_ASSERT_MSG(buffer_size > 0, "Buffer size must be greater than 0!");

        vulkan_buffer buf = {
            .m_size = buffer_size, .m_usage_flags = usage_flags, .m_memory_flags = mem_flags
        };

        const vk::BufferCreateInfo buffer_ci = {
            {}, buffer_size, usage_flags, vk::SharingMode::eExclusive
        };

        VmaAllocationCreateInfo alloc_info{};
        if (mem_flags & vk::MemoryPropertyFlagBits::eHostVisible)
        {
            alloc_info = {
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                .preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
            };
        }

        if (mem_flags & vk::MemoryPropertyFlagBits::eHostVisible)
        {
            // check if coherent buffer is available
            VK_CHECK(m_device->createBuffer(&buffer_ci, nullptr, &buf.m_buffer));
            vk::MemoryRequirements mem_reqs = m_device->getBufferMemoryRequirements(buf.m_buffer);
            m_device->destroyBuffer(buf.m_buffer);
            buf.m_buffer = VK_NULL_HANDLE;

            if (mem_reqs.memoryTypeBits & static_cast<uint8_t>(vk::MemoryPropertyFlagBits::eHostCoherent))
            {
                alloc_info.flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                buf.m_is_coherent_memory = true;
            }
        }

        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

        vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_ci), &alloc_info,
            16, reinterpret_cast<VkBuffer*>(&buf.m_buffer), &buf.m_allocation, nullptr);

        if (mem_flags & vk::MemoryPropertyFlagBits::eHostVisible)
            vmaMapMemory(m_allocator, buf.m_allocation, &buf.m_mapped_ptr);

        VK_CHECK(utils::set_debug_object_name(m_device.get(), buf.m_buffer.objectType, std::bit_cast<uint64_t>(buf.m_buffer), debug_name));

        // handle shader access
        if (usage_flags & vk::BufferUsageFlagBits::eShaderDeviceAddress)
        {
            const vk::BufferDeviceAddressInfo info{ buf.m_buffer };
            buf.m_device_address = m_device->getBufferAddress(info);
        }

        return buf;
    }

    VkDeviceAddress vk_device::get_buffer_device_address(vk::Buffer buffer) const
    {
        vk::BufferDeviceAddressInfo info{};
        info.buffer = buffer;
        return m_device->getBufferAddress(info);
    }

    vk::Sampler vk_device::allocate_sampler(vk::SamplerCreateInfo ci, const char* debug_name)
    {
        vk::Sampler sampler;
        VK_CHECK(m_device->createSampler(&ci, nullptr, &sampler));
        VK_CHECK(utils::set_debug_object_name(m_device.get(), sampler.objectType, std::bit_cast<uint64_t>(sampler), debug_name));
        return sampler;
    }

    uint32_t vk_device::get_framebuffer_msaa_bitmask() const
    {
        const auto& limits = m_physical_device_properties.limits;
        return static_cast<uint32_t>(limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts);
    }

    vk::Format vk_device::get_closest_depth_stencil_format(Format format) const
    {
        const std::vector<vk::Format> formats = get_compatible_depth_stencil_formats(format);

        std::set<vk::Format> available_formats;
        const auto& supported_depth_formats = get_supported_depth_formats();
        for (vk::Format f : supported_depth_formats)
        {
            available_formats.insert(f);
        }

        // check if any of the formats in compatible list are supported
        for (vk::Format f : formats)
        {
            if (available_formats.contains(f))
                return f;
        }

        // no matching found, choose first supported format
        return !supported_depth_formats.empty() ? supported_depth_formats[0] : vk::Format::eD24UnormS8Uint;
    }

    void vk_device::buffer_subdata(vulkan_buffer& buffer, std::size_t offset, std::size_t size, const void* data)
    {
        if (buffer.is_mapped())
        {
            buffer.buffer_subdata(*m_context, offset, size, data);
            return;
        }

        // TODO: unmapped buffer sub data
    }

    void vk_device::image_data2D(vulkan_image& image, const vk::Rect2D& image_region, uint32_t base_mip_level,
        uint32_t num_mip_levels, uint32_t layer, uint32_t num_layers, vk::Format format, const void* data)
    {
        MOON_CORE_ASSERT(num_mip_levels <= s_max_mip_levels);

#if 0
        // divide width and height by 2 until we get to the size of base mip level
        uint32_t width = image.m_extent.width >> base_mip_level;
        uint32_t height = image.m_extent.height >> base_mip_level;

        const Format tex_format = utils::vk_format_to_format(format);

        // find storage size for all mip-levels being uploaded
        uint32_t layer_storage_size = 0;
        for (uint32_t i = 0; i < num_mip_levels; ++i)
        {
        }
#endif
    }

    void vk_device::image_data3D(vulkan_image& image, const vk::Offset3D& offset,
                                 const vk::Extent3D& extent, vk::Format format, const void* data)
    {

    }

    void vk_device::get_image_data(vulkan_image& image, const vk::Offset3D& offset, const vk::Extent3D& extent,
        vk::ImageSubresourceRange range, vk::Format format, void* out_data)
    {

    }

    vk_device::memory_region_desc vk_device::get_next_free_offset(uint32_t size)
    {
        return {};
    }

    void vk_device::ensure_staging_buffer_size(uint32_t size)
    {

    }

    void vk_device::wait_and_reset()
    {}

    constexpr std::vector<vk::Format> vk_device::get_compatible_depth_stencil_formats(Format format)
    {
        switch (format)
        {
        case Format::Z_UN16:
            return { vk::Format::eD16Unorm, vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat };
        case Format::Z_UN24:
            return { vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat , vk::Format::eD16UnormS8Uint};
        case Format::Z_F32:
            return { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
        case Format::Z_UN24_S_UI8:
            return { vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint };
        case Format::Z_F32_S_UI8:
            return { vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint };
        }
        return { vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat };
    }
}
