#pragma once

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp> // consider transitioning to c++ module import

#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>

#define VK_CHECK(func)                                              \
    {                                                               \
        const vk::Result r = func;                                  \
        if (r != vk::Result::eSuccess)                              \
        {                                                           \
            MOON_CORE_ASSERT("Vulkan API call failed: {}\n {}\n",   \
                #func,                                              \
                vk::to_string(r));                                  \
        }                                                           \
    }                                                               \

namespace moon::vulkan
{
    struct allocated_image
    {
        vk::Image image;
        vk::ImageView view;
        VmaAllocation allocation;
        vk::Extent3D extent;
        vk::Format format;
    };

    inline void transition_image(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
    {
        vk::ImageMemoryBarrier2 image_barrier{};

        // when we start doing post-process chains consider using better timed stage masks
        image_barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
        image_barrier.srcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
        image_barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
        image_barrier.dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

        image_barrier.oldLayout = old_layout;
        image_barrier.newLayout = new_layout;

        vk::ImageAspectFlags aspect_mask = vk::ImageAspectFlagBits::eColor;
        if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
        {
            aspect_mask = vk::ImageAspectFlagBits::eDepth;
        }
        image_barrier.subresourceRange = vk::ImageSubresourceRange{ aspect_mask, 0, 1, 0, 1 };
        image_barrier.image = image;

        vk::DependencyInfo dep_info{};
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;

        cmd.pipelineBarrier2(&dep_info);
    }

    inline void copy_image_to_image(vk::CommandBuffer cmd, vk::Image src_image, vk::Image dst_image, vk::Extent2D src_size,
        vk::Extent2D dst_size)
    {
        vk::ImageBlit2 blit_region{};
        blit_region.srcOffsets[1] = vk::Offset3D{ (int)src_size.width, (int)src_size.height, 1 };
        blit_region.dstOffsets[1] = vk::Offset3D{ (int)dst_size.width, (int)dst_size.height, 1 };

        blit_region.srcSubresource = vk::ImageSubresourceLayers{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 };
        blit_region.dstSubresource = vk::ImageSubresourceLayers{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 };

        vk::BlitImageInfo2 blit_info{};
        blit_info.srcImage = src_image;
        blit_info.dstImage = dst_image;
        blit_info.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
        blit_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
        blit_info.filter = vk::Filter::eLinear;
        blit_info.pRegions = &blit_region;
        blit_info.regionCount = 1;

        cmd.blitImage2(&blit_info);
    }
}
