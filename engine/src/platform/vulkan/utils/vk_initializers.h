#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

// from https://github.com/KhronosGroup/Vulkan-Samples/blob/main/framework/common/vk_initializers.h
// transformed to match vulkan.hpp
namespace moon::vulkan::init
{
    constexpr inline vk::MemoryAllocateInfo memory_allocate_info()
    {
        return {};
    }

    constexpr inline vk::MappedMemoryRange mapped_memory_range()
    {
        return {};
    }

    constexpr inline vk::CommandBufferAllocateInfo command_buffer_allocate_info(
        vk::CommandPool        command_pool,
        vk::CommandBufferLevel level,
        uint32_t             buffer_count)
    {
        vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.commandPool        = command_pool;
        command_buffer_allocate_info.level              = level;
        command_buffer_allocate_info.commandBufferCount = buffer_count;
        return command_buffer_allocate_info;
    }

    constexpr inline vk::CommandPoolCreateInfo command_pool_create_info()
    {
        return {};
    }

    constexpr inline vk::CommandBufferBeginInfo command_buffer_begin_info()
    {
        return {};
    }

    constexpr inline vk::CommandBufferInheritanceInfo command_buffer_inheritance_info()
    {
        return {};
    }

    constexpr inline vk::RenderPassBeginInfo render_pass_begin_info()
    {
        return {};
    }

    constexpr inline vk::RenderPassCreateInfo render_pass_create_info()
    {
        return {};
    }

    /** @brief Initialize rendering_attachment_info */
    constexpr inline vk::RenderingAttachmentInfoKHR rendering_attachment_info()
    {
        vk::RenderingAttachmentInfoKHR attachment_info{};
        attachment_info.pNext = VK_NULL_HANDLE;
        return attachment_info;
    }

    /** @brief Initialize vk::RenderingInfoKHR, e.g. for use with dynamic rendering extension */
    constexpr inline vk::RenderingInfoKHR rendering_info(vk::Rect2D                            render_area            = {},
                                             uint32_t                            color_attachment_count = 0,
                                             const vk::RenderingAttachmentInfoKHR *pColorAttachments      = VK_NULL_HANDLE,
                                             vk::RenderingFlagsKHR                 flags                  = vk::RenderingFlagsKHR{} )
    {
        vk::RenderingInfoKHR rendering_info   = {};
        rendering_info.flags                = flags;
        rendering_info.renderArea           = render_area;
        rendering_info.layerCount           = 0;
        rendering_info.viewMask             = 0;
        rendering_info.colorAttachmentCount = color_attachment_count;
        rendering_info.pColorAttachments    = pColorAttachments;
        rendering_info.pDepthAttachment     = VK_NULL_HANDLE;
        rendering_info.pStencilAttachment   = VK_NULL_HANDLE;
        return rendering_info;
    }

    /** @brief Initialize an image memory barrier with no image transfer ownership */
    constexpr inline vk::ImageMemoryBarrier image_memory_barrier()
    {
        vk::ImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return image_memory_barrier;
    }

    /** @brief Initialize a buffer memory barrier with no image transfer ownership */
    constexpr inline vk::BufferMemoryBarrier buffer_memory_barrier()
    {
        vk::BufferMemoryBarrier buffer_memory_barrier{};
        buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return buffer_memory_barrier;
    }

    constexpr inline vk::MemoryBarrier memory_barrier()
    {
        return {};
    }

    constexpr inline vk::ImageCreateInfo image_create_info()
    {
        return {};
    }

    constexpr inline vk::SamplerCreateInfo sampler_create_info()
    {
        vk::SamplerCreateInfo sampler_create_info{};
        sampler_create_info.maxAnisotropy = 1.0f;
        return sampler_create_info;
    }

    constexpr inline vk::ImageViewCreateInfo image_view_create_info()
    {
        return {};
    }

    constexpr inline vk::FramebufferCreateInfo framebuffer_create_info()
    {
        return {};
    }

    constexpr inline vk::SemaphoreCreateInfo semaphore_create_info()
    {
        return {};
    }

    constexpr inline vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlags flags = 0)
    {
        vk::FenceCreateInfo fence_create_info{};
        fence_create_info.flags = flags;
        return fence_create_info;
    }

    constexpr inline vk::EventCreateInfo event_create_info()
    {
        return {};
    }

    constexpr inline vk::SubmitInfo submit_info()
    {
        return {};
    }

    constexpr inline vk::Viewport viewport(
        float width,
        float height,
        float min_depth,
        float max_depth)
    {
        vk::Viewport viewport{};
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = min_depth;
        viewport.maxDepth = max_depth;
        return viewport;
    }

    constexpr inline vk::Rect2D rect2D(
        int32_t width,
        int32_t height,
        int32_t offset_x,
        int32_t offset_y)
    {
        vk::Rect2D rect2D{};
        rect2D.extent.width  = width;
        rect2D.extent.height = height;
        rect2D.offset.x      = offset_x;
        rect2D.offset.y      = offset_y;
        return rect2D;
    }

    constexpr inline vk::BufferCreateInfo buffer_create_info()
    {
        return {};
    }

    constexpr inline vk::BufferCreateInfo buffer_create_info(
        vk::BufferUsageFlags usage,
        vk::DeviceSize       size)
    {
        vk::BufferCreateInfo buffer_create_info{};
        buffer_create_info.usage = usage;
        buffer_create_info.size  = size;
        return buffer_create_info;
    }

    constexpr inline vk::DescriptorPoolCreateInfo descriptor_pool_create_info(
        uint32_t              count,
        vk::DescriptorPoolSize *pool_sizes,
        uint32_t              max_sets)
    {
        vk::DescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.poolSizeCount = count;
        descriptor_pool_info.pPoolSizes    = pool_sizes;
        descriptor_pool_info.maxSets       = max_sets;
        return descriptor_pool_info;
    }

    constexpr inline vk::DescriptorPoolCreateInfo descriptor_pool_create_info(
        const std::vector<vk::DescriptorPoolSize> &pool_sizes,
        uint32_t                                 max_sets)
    {
        vk::DescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes    = pool_sizes.data();
        descriptor_pool_info.maxSets       = max_sets;
        return descriptor_pool_info;
    }

    constexpr inline vk::DescriptorPoolSize descriptor_pool_size(
        vk::DescriptorType type,
        uint32_t         count)
    {
        vk::DescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type            = type;
        descriptor_pool_size.descriptorCount = count;
        return descriptor_pool_size;
    }

    constexpr inline vk::DescriptorSetLayoutBinding descriptor_set_layout_binding(
        vk::DescriptorType   type,
        vk::ShaderStageFlags flags,
        uint32_t           binding,
        uint32_t           count = 1)
    {
        vk::DescriptorSetLayoutBinding set_layout_binding{};
        set_layout_binding.descriptorType  = type;
        set_layout_binding.stageFlags      = flags;
        set_layout_binding.binding         = binding;
        set_layout_binding.descriptorCount = count;
        return set_layout_binding;
    }

    constexpr inline vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(
        const vk::DescriptorSetLayoutBinding *bindings,
        uint32_t                            binding_count)
    {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.pBindings    = bindings;
        descriptor_set_layout_create_info.bindingCount = binding_count;
        return descriptor_set_layout_create_info;
    }

    constexpr inline vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(
        const std::vector<vk::DescriptorSetLayoutBinding> &bindings)
    {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.pBindings    = bindings.data();
        descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
        return descriptor_set_layout_create_info;
    }

    constexpr inline vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
        const vk::DescriptorSetLayout *set_layouts,
        uint32_t                     set_layout_count = 1)
    {
        vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.setLayoutCount = set_layout_count;
        pipeline_layout_create_info.pSetLayouts    = set_layouts;
        return pipeline_layout_create_info;
    }

    constexpr inline vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
        uint32_t set_layout_count = 1)
    {
        vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.setLayoutCount = set_layout_count;
        return pipeline_layout_create_info;
    }

    constexpr inline vk::DescriptorSetAllocateInfo descriptor_set_allocate_info(
        vk::DescriptorPool             descriptor_pool,
        const vk::DescriptorSetLayout *set_layouts,
        uint32_t                     descriptor_set_count)
    {
        vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.descriptorPool     = descriptor_pool;
        descriptor_set_allocate_info.pSetLayouts        = set_layouts;
        descriptor_set_allocate_info.descriptorSetCount = descriptor_set_count;
        return descriptor_set_allocate_info;
    }

    constexpr inline vk::DescriptorImageInfo descriptor_image_info(vk::Sampler sampler, vk::ImageView image_view, vk::ImageLayout image_layout)
    {
        vk::DescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler     = sampler;
        descriptor_image_info.imageView   = image_view;
        descriptor_image_info.imageLayout = image_layout;
        return descriptor_image_info;
    }

    constexpr inline vk::WriteDescriptorSet write_descriptor_set(
        vk::DescriptorSet         dst_set,
        vk::DescriptorType        type,
        uint32_t                binding,
        vk::DescriptorBufferInfo *buffer_info,
        uint32_t                descriptor_count = 1)
    {
        vk::WriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.dstSet          = dst_set;
        write_descriptor_set.descriptorType  = type;
        write_descriptor_set.dstBinding      = binding;
        write_descriptor_set.pBufferInfo     = buffer_info;
        write_descriptor_set.descriptorCount = descriptor_count;
        return write_descriptor_set;
    }

    constexpr inline vk::WriteDescriptorSet write_descriptor_set(
        vk::DescriptorSet        dst_set,
        vk::DescriptorType       type,
        uint32_t               binding,
        vk::DescriptorImageInfo *image_info,
        uint32_t               descriptor_count = 1)
    {
        vk::WriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.dstSet          = dst_set;
        write_descriptor_set.descriptorType  = type;
        write_descriptor_set.dstBinding      = binding;
        write_descriptor_set.pImageInfo      = image_info;
        write_descriptor_set.descriptorCount = descriptor_count;
        return write_descriptor_set;
    }

    constexpr inline vk::VertexInputBindingDescription vertex_input_binding_description(
        uint32_t          binding,
        uint32_t          stride,
        vk::VertexInputRate input_rate)
    {
        vk::VertexInputBindingDescription vertex_input_binding_description{};
        vertex_input_binding_description.binding   = binding;
        vertex_input_binding_description.stride    = stride;
        vertex_input_binding_description.inputRate = input_rate;
        return vertex_input_binding_description;
    }

    constexpr inline vk::VertexInputBindingDescription2EXT vertex_input_binding_description2ext(
        uint32_t          binding,
        uint32_t          stride,
        vk::VertexInputRate input_rate,
        uint32_t          divisor)
    {
        vk::VertexInputBindingDescription2EXT vertex_input_binding_description2ext{};
        vertex_input_binding_description2ext.binding   = binding;
        vertex_input_binding_description2ext.stride    = stride;
        vertex_input_binding_description2ext.inputRate = input_rate;
        vertex_input_binding_description2ext.divisor   = divisor;
        return vertex_input_binding_description2ext;
    }

    constexpr inline vk::VertexInputAttributeDescription vertex_input_attribute_description(
        uint32_t binding,
        uint32_t location,
        vk::Format format,
        uint32_t offset)
    {
        vk::VertexInputAttributeDescription vertex_input_attribute_description{};
        vertex_input_attribute_description.location = location;
        vertex_input_attribute_description.binding  = binding;
        vertex_input_attribute_description.format   = format;
        vertex_input_attribute_description.offset   = offset;
        return vertex_input_attribute_description;
    }

    constexpr inline vk::VertexInputAttributeDescription2EXT vertex_input_attribute_description2ext(
        uint32_t binding,
        uint32_t location,
        vk::Format format,
        uint32_t offset)
    {
        vk::VertexInputAttributeDescription2EXT vertex_input_attribute_description2ext{};
        vertex_input_attribute_description2ext.location = location;
        vertex_input_attribute_description2ext.binding  = binding;
        vertex_input_attribute_description2ext.format   = format;
        vertex_input_attribute_description2ext.offset   = offset;
        return vertex_input_attribute_description2ext;
    }

    constexpr inline vk::PipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info()
    {
        vk::PipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
        return pipeline_vertex_input_state_create_info;
    }

    constexpr inline vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info(
        vk::PrimitiveTopology                     topology,
        vk::PipelineInputAssemblyStateCreateFlags flags,
        vk::Bool32                                primitive_restart_enable)
    {
        vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.topology               = topology;
        pipeline_input_assembly_state_create_info.flags                  = flags;
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = primitive_restart_enable;
        return pipeline_input_assembly_state_create_info;
    }

    constexpr inline vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info(
        vk::PolygonMode                           polygon_mode,
        vk::CullModeFlags                         cull_mode,
        vk::FrontFace                             front_face,
        vk::PipelineRasterizationStateCreateFlags flags = vk::PipelineRasterizationStateCreateFlags{})
    {
        vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
        pipeline_rasterization_state_create_info.polygonMode      = polygon_mode;
        pipeline_rasterization_state_create_info.cullMode         = cull_mode;
        pipeline_rasterization_state_create_info.frontFace        = front_face;
        pipeline_rasterization_state_create_info.flags            = flags;
        pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.lineWidth        = 1.0f;
        return pipeline_rasterization_state_create_info;
    }

    constexpr inline vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state(
        vk::ColorComponentFlags color_write_mask,
        vk::Bool32              blend_enable)
    {
        vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{};
        pipeline_color_blend_attachment_state.colorWriteMask = color_write_mask;
        pipeline_color_blend_attachment_state.blendEnable    = blend_enable;
        return pipeline_color_blend_attachment_state;
    }

    constexpr inline vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info(
        uint32_t                                   attachment_count,
        const vk::PipelineColorBlendAttachmentState *attachments)
    {
        vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
        pipeline_color_blend_state_create_info.attachmentCount = attachment_count;
        pipeline_color_blend_state_create_info.pAttachments    = attachments;
        return pipeline_color_blend_state_create_info;
    }

    constexpr inline vk::PipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info(
        vk::Bool32    depth_test_enable,
        vk::Bool32    depth_write_enable,
        vk::CompareOp depth_compare_op)
    {
        vk::PipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
        pipeline_depth_stencil_state_create_info.depthTestEnable  = depth_test_enable;
        pipeline_depth_stencil_state_create_info.depthWriteEnable = depth_write_enable;
        pipeline_depth_stencil_state_create_info.depthCompareOp   = depth_compare_op;
        pipeline_depth_stencil_state_create_info.front            = pipeline_depth_stencil_state_create_info.back;
        pipeline_depth_stencil_state_create_info.back.compareOp   = vk::CompareOp::eAlways;
        return pipeline_depth_stencil_state_create_info;
    }

    constexpr inline vk::PipelineViewportStateCreateInfo pipeline_viewport_state_create_info(
        uint32_t                           viewport_count,
        uint32_t                           scissor_count,
        vk::PipelineViewportStateCreateFlags flags = vk::PipelineViewportStateCreateFlags{})
    {
        vk::PipelineViewportStateCreateInfo pipeline_viewport_state_create_info{};
        pipeline_viewport_state_create_info.viewportCount = viewport_count;
        pipeline_viewport_state_create_info.scissorCount  = scissor_count;
        pipeline_viewport_state_create_info.flags         = flags;
        return pipeline_viewport_state_create_info;
    }

    constexpr inline vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info(
        vk::SampleCountFlagBits                 rasterization_samples,
        vk::PipelineMultisampleStateCreateFlags flags = vk::PipelineMultisampleStateCreateFlags{})
    {
        vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
        pipeline_multisample_state_create_info.rasterizationSamples = rasterization_samples;
        pipeline_multisample_state_create_info.flags                = flags;
        return pipeline_multisample_state_create_info;
    }

    constexpr inline vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info(
        const vk::DynamicState             *dynamic_states,
        uint32_t                          dynamicStateCount,
        vk::PipelineDynamicStateCreateFlags flags = vk::PipelineDynamicStateCreateFlags{})
    {
        vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
        pipeline_dynamic_state_create_info.pDynamicStates    = dynamic_states;
        pipeline_dynamic_state_create_info.dynamicStateCount = dynamicStateCount;
        pipeline_dynamic_state_create_info.flags             = flags;
        return pipeline_dynamic_state_create_info;
    }

    constexpr inline vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info(
        const std::vector<vk::DynamicState> &dynamic_states,
        vk::PipelineDynamicStateCreateFlags  flags = vk::PipelineDynamicStateCreateFlags{})
    {
        return { flags, dynamic_states };
    }

    constexpr inline vk::PipelineTessellationStateCreateInfo pipeline_tessellation_state_create_info(uint32_t patch_control_points)
    {
        vk::PipelineTessellationStateCreateInfo pipeline_tessellation_state_create_info{};
        pipeline_tessellation_state_create_info.patchControlPoints = patch_control_points;
        return pipeline_tessellation_state_create_info;
    }

    constexpr inline vk::GraphicsPipelineCreateInfo pipeline_create_info(
        vk::PipelineLayout      layout,
        vk::RenderPass          render_pass,
        vk::PipelineCreateFlags flags = vk::PipelineCreateFlags{})
    {
        vk::GraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.layout             = layout;
        pipeline_create_info.renderPass         = render_pass;
        pipeline_create_info.flags              = flags;
        pipeline_create_info.basePipelineIndex  = -1;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        return pipeline_create_info;
    }

    constexpr inline vk::GraphicsPipelineCreateInfo pipeline_create_info()
    {
        vk::GraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.basePipelineIndex  = -1;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        return pipeline_create_info;
    }

    constexpr inline vk::ComputePipelineCreateInfo compute_pipeline_create_info(
        vk::PipelineLayout      layout,
        vk::PipelineCreateFlags flags = vk::PipelineCreateFlags{})
    {
        vk::ComputePipelineCreateInfo compute_pipeline_create_info{};
        compute_pipeline_create_info.layout = layout;
        compute_pipeline_create_info.flags  = flags;
        return compute_pipeline_create_info;
    }

    constexpr inline vk::PushConstantRange push_constant_range(
        vk::ShaderStageFlags stage_flags,
        uint32_t           size,
        uint32_t           offset)
    {
        vk::PushConstantRange push_constant_range{};
        push_constant_range.stageFlags = stage_flags;
        push_constant_range.offset     = offset;
        push_constant_range.size       = size;
        return push_constant_range;
    }

    constexpr inline vk::BindSparseInfo bind_sparse_info()
    {
        return {};
    }

    /** @brief Initialize a map entry for a shader specialization constant */
    constexpr inline vk::SpecializationMapEntry specialization_map_entry(uint32_t constant_id, uint32_t offset, size_t size)
    {
        vk::SpecializationMapEntry specialization_map_entry{};
        specialization_map_entry.constantID = constant_id;
        specialization_map_entry.offset     = offset;
        specialization_map_entry.size       = size;
        return specialization_map_entry;
    }

    /** @brief Initialize a specialization constant info structure to pass to a shader stage */
    constexpr inline vk::SpecializationInfo specialization_info(uint32_t map_entry_count, const vk::SpecializationMapEntry *map_entries, size_t data_size, const void *data)
    {
        vk::SpecializationInfo specialization_info{};
        specialization_info.mapEntryCount = map_entry_count;
        specialization_info.pMapEntries   = map_entries;
        specialization_info.dataSize      = data_size;
        specialization_info.pData         = data;
        return specialization_info;
    }

    constexpr inline vk::TimelineSemaphoreSubmitInfo timeline_semaphore_submit_info(uint32_t wait_value_count, uint64_t *wait_values, uint32_t signal_value_count, uint64_t *signal_values)
    {
        return vk::TimelineSemaphoreSubmitInfo{wait_value_count, wait_values,
            signal_value_count, signal_values};
    }
}
