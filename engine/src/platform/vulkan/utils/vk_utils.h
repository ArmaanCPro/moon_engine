#pragma once

#include <vulkan/vulkan.hpp>
#include "vulkan/vk.h"

#include "core/log.h"
#include "renderer/render_types.h"
#include "vulkan/vk_render_types.h"

struct moon::vulkan::vulkan_image;

namespace moon::vulkan::utils
{
    [[nodiscard]] vk::Result set_debug_object_name(vk::Device device, vk::ObjectType type, uint64_t object, const char* name);

    [[nodiscard]] vk::Semaphore create_semaphore(vk::Device device, const char* debug_name = nullptr);

    [[nodiscard]] vk::Semaphore create_timeline_semaphore(vk::Device device, uint64_t initial_value, const char* debug_name = nullptr);

    [[nodiscard]] vk::Fence create_fence(vk::Device device, const char* debug_name = nullptr);

    [[nodiscard]] uint32_t find_queue_family_index(vk::PhysicalDevice physical_device, vk::QueueFlags flags);


    // conversion helper functions
    // ---------------------------

    [[nodiscard]] constexpr vk::Filter sampler_filter_to_vk_filter(SamplerFilter filter);

    [[nodiscard]] constexpr vk::SamplerMipmapMode sampler_mipmap_to_vk_sampler_mipmap_mode(SamplerMip filter);

    [[nodiscard]] constexpr vk::SamplerAddressMode sampler_wrap_mode_to_vk_sampler_address_mode(SamplerWrap mode);

    [[nodiscard]] constexpr vk::CompareOp compare_op_to_vk_compare_op(CompareOp op);


    [[nodiscard]] vk::SamplerCreateInfo sampler_state_desc_to_vk_sampler_create_info(const sampler_state_desc& desc,
                                                                                     const vk::PhysicalDeviceLimits& limits);

    [[nodiscard]] vk::SpecializationInfo get_pipeline_shader_stage_specialization_info(specialization_constant_desc desc,
                                                                                       vk::SpecializationMapEntry* out_entries);

    struct stage_access
    {
        vk::PipelineStageFlags2 stage;
        vk::AccessFlags2 access;
    };

    [[nodiscard]] constexpr stage_access get_pipeline_stage_access(vk::ImageLayout layout);

    void image_memory_barrier_2(vk::CommandBuffer cmd, vk::Image image, stage_access src, stage_access dst,
                                vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageSubresourceRange subresource_range);

    inline void transition_to_color_attachment(vk::CommandBuffer cmd, vulkan_image* color_tex);

    [[nodiscard]] constexpr bool is_depth_or_stencil_vk_format(vk::Format format);
    [[nodiscard]] constexpr vk::IndexType index_format_to_vk_index_type(IndexFormat format);
    [[nodiscard]] constexpr vk::PrimitiveTopology topology_to_vk_primitive_topology(Topology top);
    [[nodiscard]] constexpr vk::AttachmentLoadOp load_op_to_vk_attachment_load_op(LoadOp op);
    [[nodiscard]] constexpr vk::AttachmentStoreOp store_op_to_vk_attachment_store_op(StoreOp op);
    [[nodiscard]] constexpr vk::ShaderStageFlagBits shader_stage_to_vk_shader_stage(ShaderStage stage);
    [[nodiscard]] constexpr vk::MemoryPropertyFlags storage_type_to_vk_memory_property_flags(StorageType type);
    [[nodiscard]] constexpr vk::BuildAccelerationStructureFlagsKHR build_flags_to_vk_build_acceleration_structure_flags(uint8_t build_flags);
    [[nodiscard]] constexpr vk::PolygonMode polygon_mode_to_vk_polygon_mode(PolygonMode mode);
    [[nodiscard]] constexpr vk::BlendFactor blend_factor_to_vk_blend_factor(BlendFactor factor);
    [[nodiscard]] constexpr vk::BlendOp blend_op_to_vk_blend_op(BlendOp op);
    [[nodiscard]] constexpr vk::CullModeFlags cull_mode_to_vk_cull_mode(CullMode mode);
    [[nodiscard]] constexpr vk::FrontFace winding_mode_vk_front_face(WindingMode mode);
    [[nodiscard]] constexpr vk::StencilOp stencil_op_to_vk_stencil_op(StencilOp op);
    [[nodiscard]] constexpr vk::Format vertex_format_to_vk_format(VertexFormat format);

    [[nodiscard]] constexpr uint32_t get_bytes_per_pixel(vk::Format format);
}
