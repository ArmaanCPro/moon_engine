#include "moonpch.h"
#include "vk_utils.h"

#include "vulkan/vk_render_types.h"

namespace moon::vulkan::utils
{
    vk::Result set_debug_object_name(vk::Device device, vk::ObjectType type, uint64_t object, const char* name)
    {
        if (!name || !*name)
            return vk::Result::eSuccess;

        const vk::DebugUtilsObjectNameInfoEXT info{type, object, name};
        return device.setDebugUtilsObjectNameEXT(&info);
    }

    vk::Semaphore create_semaphore(vk::Device device, const char* debug_name)
    {
        constexpr vk::SemaphoreCreateInfo ci{};
        vk::Semaphore semaphore;
        VK_CHECK(device.createSemaphore(&ci, nullptr, &semaphore));

        if (debug_name && *debug_name)
        {
            VK_CHECK(set_debug_object_name(device, semaphore.objectType, std::bit_cast<uint64_t>(semaphore.operator VkSemaphore()), debug_name));
        }
        return semaphore;
    }

    vk::Semaphore create_timeline_semaphore(vk::Device device, uint64_t initial_value, const char* debug_name)
    {
        const vk::SemaphoreTypeCreateInfo timeline_semaphore_type_info{vk::SemaphoreType::eTimeline, initial_value};
        const vk::SemaphoreCreateInfo ci{vk::SemaphoreCreateFlags(), &timeline_semaphore_type_info};
        vk::Semaphore semaphore;
        VK_CHECK(device.createSemaphore(&ci, nullptr, &semaphore));

        if (debug_name)
        {
            VK_CHECK(set_debug_object_name(device, semaphore.objectType, std::bit_cast<uint64_t>(semaphore.operator VkSemaphore()), debug_name));
        }
        return semaphore;
    }

    vk::Fence create_fence(vk::Device device, const char* debug_name)
    {
        constexpr vk::FenceCreateInfo ci{};
        vk::Fence fence;
        VK_CHECK(device.createFence(&ci, nullptr, &fence));
        if (debug_name)
        {
            VK_CHECK(set_debug_object_name(device, fence.objectType, std::bit_cast<uint64_t>(fence.operator VkFence()), debug_name));
        }
        return fence;
    }

    uint32_t find_queue_family_index(vk::PhysicalDevice physical_device, vk::QueueFlags flags)
    {
        static constexpr uint32_t s_invalid_queue_index = std::numeric_limits<uint32_t>::max();

        std::vector<vk::QueueFamilyProperties> props = physical_device.getQueueFamilyProperties();
        auto find_dedicated_queue_family_index = [&props](vk::QueueFlags require, vk::QueueFlags exclude) -> uint32_t
        {
            for (uint32_t i = 0; i != props.size(); ++i)
            {
                const bool suitable = (props[i].queueFlags & require) == require;
                const bool dedicated = (props[i].queueFlags & exclude) == vk::QueueFlags{};
                if (props[i].queueCount > 0 && suitable && dedicated)
                    return i;
            }
            return s_invalid_queue_index;
        };

        // dedicated queue for compute
        if (flags & vk::QueueFlagBits::eCompute)
        {
            const uint32_t q = find_dedicated_queue_family_index(vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
            if (q != s_invalid_queue_index)
                return q;
        }

        // dedicated queue for transfer
        if (flags & vk::QueueFlagBits::eTransfer)
        {
            const uint32_t q = find_dedicated_queue_family_index(vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics);
            if (q != s_invalid_queue_index)
                return q;
        }

        // any suitable queue
        return find_dedicated_queue_family_index(flags, vk::QueueFlagBits{});
    }

    vk::SamplerCreateInfo sampler_state_desc_to_vk_sampler_create_info(const sampler_state_desc& desc,
                                                                       const vk::PhysicalDeviceLimits& limits)
    {
        MOON_CORE_ASSERT(desc.mipLodMax >= desc.mipLodMin);

        vk::SamplerCreateInfo sampler_info{
            vk::SamplerCreateFlags{},
            sampler_filter_to_vk_filter(desc.mag_filter),
            sampler_filter_to_vk_filter(desc.min_filter),
            sampler_mipmap_to_vk_sampler_mipmap_mode(desc.mip_map),
            sampler_wrap_mode_to_vk_sampler_address_mode(desc.wrap_u),
            sampler_wrap_mode_to_vk_sampler_address_mode(desc.wrap_v),
            sampler_wrap_mode_to_vk_sampler_address_mode(desc.wrap_w),
            0.0f,
            vk::False,
            0.0f,
            desc.depth_compare_enabled ? vk::True : vk::False,
            desc.depth_compare_enabled ? compare_op_to_vk_compare_op(desc.depth_compare_op) : vk::CompareOp::eAlways,
            static_cast<float>(desc.mipLodMax),
            desc.mip_map == SamplerMip::Disabled ? static_cast<float>(desc.mipLodMin) : static_cast<float>(desc.mipLodMax),
            vk::BorderColor::eIntOpaqueBlack,
            vk::False
        };

        if (desc.max_anisotropic > 1)
        {
            const bool anisotropy_supported = limits.maxSamplerAnisotropy > 1;
            MOON_CORE_ASSERT_MSG(anisotropy_supported, "Anisotropic filtering is not supported on this device!");
            sampler_info.anisotropyEnable = anisotropy_supported ? vk::True : vk::False;

            if (limits.maxSamplerAnisotropy < desc.max_anisotropic)
            {
                MOON_CORE_WARN("Supplied sampler anisotropic value greater than max supported by device, setting to {}",
                               limits.maxSamplerAnisotropy);
            }
            sampler_info.maxAnisotropy = std::min(limits.maxSamplerAnisotropy, static_cast<float>(desc.max_anisotropic));
        }
        return sampler_info;
    }

    vk::SpecializationInfo get_pipeline_shader_stage_specialization_info(specialization_constant_desc desc,
                                                                         vk::SpecializationMapEntry* out_entries)
    {
        const uint32_t num_entries = desc.get_num_specialization_constants();
        if (out_entries)
        {
            for (uint32_t i = 0; i != num_entries; ++i)
            {
                out_entries[i] = vk::SpecializationMapEntry{ desc.entries[i].constant_id, desc.entries[i].offset, desc.entries[i].size };
            }
        }
        return vk::SpecializationInfo{ num_entries, out_entries, desc.data_size, desc.data };
    }

    void image_memory_barrier_2(vk::CommandBuffer cmd, vk::Image image, stage_access src, stage_access dst,
                                vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageSubresourceRange subresource_range)
    {
        const vk::ImageMemoryBarrier2 image_barrier {
            src.stage, src.access,
            dst.stage, dst.access,
            old_layout, new_layout,
            0, 0,
            image,
            subresource_range
        };

        vk::DependencyInfo dep_info{};
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;

        cmd.pipelineBarrier2(&dep_info);
    }

    void transition_to_color_attachment(vk::CommandBuffer cmd, vulkan_image* color_tex)
    {
        if (!color_tex)
            return;

        if (!color_tex->m_is_depth_format || !color_tex->m_is_stencil_format)
        {
            MOON_CORE_ASSERT_MSG(false, "Color attachments cannot have depth/stencil format!");
            return;
        }
        MOON_CORE_ASSERT_MSG(color_tex->m_format != vk::Format::eUndefined, "Invalid color attachment format!");
        color_tex->transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal,
            { vk::ImageAspectFlagBits::eColor, 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers });
    }
}
