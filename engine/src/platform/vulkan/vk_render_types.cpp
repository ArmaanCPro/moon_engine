#include "moonpch.h"
#include "vulkan/vk_render_types.h"

#include "utils/vk_utils.h"
#include "vulkan/vk_context.h"

namespace moon::vulkan
{
    void vulkan_buffer::buffer_subdata(vk_context& context, std::size_t offset, std::size_t size,
        const void* data)
    {
        if (!m_mapped_ptr)
            return;

        MOON_CORE_ASSERT_MSG(offset + size <= m_size, "Offset + size must be less than buffer size");

        if (data)
            memcpy((uint8_t*)m_mapped_ptr + offset, data, size);
        else
            memset((uint8_t*)m_mapped_ptr + offset, 0, size);

        if (!m_is_coherent_memory)
            flush_mapped_memory(context, offset, size);
    }

    void vulkan_buffer::get_buffer_subdata(vk_context& context, std::size_t offset, std::size_t size, void* data)
    {
        MOON_CORE_ASSERT_MSG(m_mapped_ptr, "Buffer is not mapped");

        if (!m_mapped_ptr)
            return;

        MOON_CORE_ASSERT_MSG(offset + size <= m_size, "Offset + size must be less than buffer size");

        if (!m_is_coherent_memory)
            invalidate_mapped_memory(context, offset, size);

        const uint8_t* src = (const uint8_t*)m_mapped_ptr + offset;
        memcpy(data, src, size);
    }

    void vulkan_buffer::flush_mapped_memory(const vk_context& context, vk::DeviceSize offset, vk::DeviceSize size) const
    {
        if (!m_mapped_ptr)
            return;

        vmaFlushAllocation(context.get_device().get_allocator(), m_allocation, offset, size);
    }

    void vulkan_buffer::invalidate_mapped_memory(const vk_context& context, vk::DeviceSize offset, vk::DeviceSize size) const
    {
        if (!m_mapped_ptr)
            return;

        vmaInvalidateAllocation(context.get_device().get_allocator(), m_allocation, offset, size);
    }

    vk::ImageView vulkan_image::create_image_view(vk::Device device, vk::ImageViewType type, vk::Format format,
        vk::ImageAspectFlags aspect_mask, uint32_t base_level, uint32_t num_levels, uint32_t base_layer,
        uint32_t num_layers, const vk::ComponentMapping mapping, const vk::SamplerYcbcrConversionInfo* ycbcr,
        const char* debug_name) const
    {
        const vk::ImageViewCreateInfo ci { {}, m_image, type, format, mapping,
            { aspect_mask, base_level, num_levels ? num_levels : m_num_levels, base_layer, num_layers} };
        vk::ImageView view;
        VK_CHECK(device.createImageView(&ci, nullptr, &view));
        VK_CHECK(utils::set_debug_object_name(device, view.objectType, std::bit_cast<uint64_t>(view.operator VkImageView()), debug_name));
        return view;
    }

    void vulkan_image::generate_mipmap(vk::CommandBuffer cmd) const
    {
        // check if device supports downscaling for color or depth/stencil buffer based on image format
        {
            constexpr vk::FormatFeatureFlags format_feature_mask = (vk::FormatFeatureFlagBits::eBlitSrc | vk::FormatFeatureFlagBits::eBlitDst);
            const bool hardware_downscaling_supported = (m_format_properties.optimalTilingFeatures & format_feature_mask) == format_feature_mask;

            if (!hardware_downscaling_supported)
            {
                MOON_CORE_WARN("Hardware downscaling not supported for image format");
                return;
            }
        }

        // choose linear filter for color formats if supported by the device, else use nearest filter
        // choose nearest filter by default for depth/stencil buffers
        const vk::Filter blit_filter = [](bool is_depth_or_stencil_format, bool image_filter_linear)
        {
            if (is_depth_or_stencil_format)
                return vk::Filter::eNearest;
            if (image_filter_linear)
                return vk::Filter::eLinear;
            return vk::Filter::eNearest;
        }(m_is_depth_format || m_is_stencil_format, (m_format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear) == vk::FormatFeatureFlagBits::eSampledImageFilterLinear);

        const vk::ImageAspectFlags image_aspect_flags = get_image_aspect_flags();

        const vk::DebugUtilsLabelEXT utils_label { "Generate mipmap", {1.0f, 0.75f, 1.0f, 1.0f} };
        cmd.beginDebugUtilsLabelEXT(&utils_label);

        const vk::ImageLayout original_image_layout = m_image_layout;

        MOON_CORE_ASSERT(original_image_layout != vk::ImageLayout::eUndefined);

        // 0: transition the first level and all layers into TransferSrcOptimal
        transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal, {image_aspect_flags, 0, 1, 0, m_num_layers});

        for (uint32_t layer = 0; layer < m_num_layers; ++layer)
        {
            int32_t mip_width = (int32_t)m_extent.width;
            int32_t mip_height = (int32_t)m_extent.height;

            for (uint32_t i = 1; i < m_num_levels; ++i)
            {
                // 1: Transition the i-th level to TransferDstOptimal; it will be copied into from the (i - 1)-th level
                utils::image_memory_barrier_2(cmd, m_image,
                    { vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone },
                    { vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite },
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                    { image_aspect_flags, i, 1, layer, 1});

                const int32_t next_level_width = mip_width > 1 ? mip_width / 2 : 1;
                const int32_t next_level_height = mip_height > 1 ? mip_height / 2 : 1;

                const vk::Offset3D src_offsets[2] = {
                    {0, 0, 0},
                    {mip_width, mip_height, 1}
                };
                const vk::Offset3D dst_offsets[2] = {
                    {0, 0, 0},
                    {next_level_width, next_level_height, 1}
                };

                // 2: Blit the image from the previous mip-level (i-1) (TransferSrcOptimal)
                // to the current mip-level (i) (TransferDstOptimal)
                const vk::ImageBlit blit {
                    vk::ImageSubresourceLayers{ image_aspect_flags, i - 1, layer, 1 },
                    { src_offsets[0], src_offsets[1] },
                    vk::ImageSubresourceLayers{ image_aspect_flags, i, layer, 1 },
                    { dst_offsets[0], dst_offsets[1] }
                };
                cmd.blitImage(m_image, vk::ImageLayout::eTransferSrcOptimal, m_image, vk::ImageLayout::eTransferDstOptimal,
                    1, &blit, blit_filter);

                // set size of next mip-level
                mip_width = next_level_width;
                mip_height = next_level_height;
            }
        }

        // 4: Transition all levels and layers (faces) to their final layout
        utils::image_memory_barrier_2(cmd, m_image, { vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead },
            { vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            vk::ImageLayout::eTransferDstOptimal, original_image_layout,
            { image_aspect_flags, 0, m_num_levels, 0, m_num_layers });

        cmd.endDebugUtilsLabelEXT();

        m_image_layout = original_image_layout;
    }

    void vulkan_image::transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout,
                                         const vk::ImageSubresourceRange& subresource_range) const
    {
        const vk::ImageLayout old_image_layout = m_image_layout == vk::ImageLayout::eAttachmentOptimal
            ? (is_depth_attachment() ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal)
            : m_image_layout;

        if (new_layout == vk::ImageLayout::eAttachmentOptimal)
            new_layout = is_depth_attachment() ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;

        utils::stage_access src = utils::get_pipeline_stage_access(old_image_layout);
        utils::stage_access dst = utils::get_pipeline_stage_access(new_layout);

        if (is_depth_attachment() && m_is_resolve_attachment)
        {
            // https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#renderpass-resolve-operations
            src.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            dst.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            src.access |= vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
            dst.access |= vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
        }

        const vk::ImageMemoryBarrier2 barrier {
            src.stage, src.access, dst.stage, dst.access,
            old_image_layout, new_layout,
            vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
            m_image,
            subresource_range
        };

        vk::DependencyInfo dep_info;
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &barrier;

        cmd.pipelineBarrier2(&dep_info);
        m_image_layout = new_layout;
    }

    vk::ImageAspectFlags vulkan_image::get_image_aspect_flags() const
    {
        vk::ImageAspectFlags flags = {};

        flags |= m_is_depth_format ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits{};
        flags |= m_is_stencil_format ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits{};
        flags |= !(m_is_depth_format || m_is_stencil_format) ? vk::ImageAspectFlagBits::eColor : vk::ImageAspectFlagBits{};

        return flags;
    }

    vk::ImageView vulkan_image::get_or_create_image_view_for_framebuffer(vk_context& context, uint32_t level,
        uint32_t layer)
    {
        MOON_CORE_ASSERT(level < s_max_mip_levels);
        MOON_CORE_ASSERT(layer < m_image_view_for_framebuffer[0].size());

        if (level >= s_max_mip_levels || layer >= m_image_view_for_framebuffer[0].size())
            return VK_NULL_HANDLE;

        if (m_image_view_for_framebuffer[level][layer] != VK_NULL_HANDLE)
            return m_image_view_for_framebuffer[level][layer];

        char debug_name_image_view[320] = {};
        std::snprintf(debug_name_image_view, sizeof(debug_name_image_view) - 1, "Image View: %s (level %u, layer %u)", m_debug_name, level, layer);

        m_image_view_for_framebuffer[level][layer] = create_image_view(context.get_device().get_device(),
            vk::ImageViewType::e2D, m_format, get_image_aspect_flags(), level, 1u, layer, 1u, {}, nullptr, debug_name_image_view);

        return m_image_view_for_framebuffer[level][layer];
    }
}
