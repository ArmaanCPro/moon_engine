#pragma once

#include "vk.h"
#include "renderer/render_types.h"

namespace moon::vulkan
{
    struct render_pipeline_state final
    {
        render_pipeline_desc desc;

        uint32_t num_bindings = 0;
        uint32_t num_attributes = 0;
        std::array<vk::VertexInputBindingDescription, vertex_input::s_vertex_buffer_max> bindings = {};
        std::array<vk::VertexInputAttributeDescription, vertex_input::s_vertex_attribute_max> attributes = {};

        // non-owning, last seen vk::DescriptorSetLayout from vk_context::m_dsl (if the context has a new layout, invalidate all vk::Pipeline objects)
        vk::DescriptorSetLayout last_descriptor_set_layout = VK_NULL_HANDLE;

        vk::ShaderStageFlags shader_stage_flags = {};
        vk::PipelineLayout pipeline_layout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;

        void* spec_constant_data_storage = nullptr;
    };

    class vk_pipeline_builder final
    {
    public:
        vk_pipeline_builder();
        ~vk_pipeline_builder() = default;

        vk_pipeline_builder& dynamic_state(vk::DynamicState state);
        vk_pipeline_builder& primitive_topology(vk::PrimitiveTopology topology);
        vk_pipeline_builder& rasterization_samples(vk::SampleCountFlagBits samples, float min_sample_shading);
        vk_pipeline_builder& shader_stage(vk::PipelineShaderStageCreateInfo stage);
        vk_pipeline_builder& stencil_state_ops(vk::StencilFaceFlags face_mask,
            vk::StencilOp failOp, vk::StencilOp passOp, vk::StencilOp depthFailOp, vk::CompareOp compareOp);
        vk_pipeline_builder& stencil_masks(vk::StencilFaceFlags face_mask, uint32_t compare_mask, uint32_t write_mask, uint32_t reference);
        vk_pipeline_builder& cull_mode(vk::CullModeFlags mode);
        vk_pipeline_builder& front_face(vk::FrontFace mode);
        vk_pipeline_builder& polygon_mode(vk::PolygonMode mode);
        vk_pipeline_builder& vertex_input_state(const vk::PipelineVertexInputStateCreateInfo& state);
        vk_pipeline_builder& color_attachments(const vk::PipelineColorBlendAttachmentState* states,
            const vk::Format* formats, uint32_t num_color_attachments);
        vk_pipeline_builder& depth_attachment_format(vk::Format format);
        vk_pipeline_builder& stencil_attachment_format(vk::Format format);
        vk_pipeline_builder& patch_control_points(uint32_t num_points);

        vk::Pipeline build(vk::Device device, vk::PipelineCache pipeline_cache, vk::PipelineLayout pipeline_layout,
            const char* debug_name = nullptr) noexcept;

        static uint32_t get_num_pipelines_created() { return s_num_pipelines_created; }

    private:
        static constexpr auto s_max_dynamic_states = 128u;
        uint32_t m_num_dynamic_states = 0;
        vk::DynamicState m_dynamic_states[s_max_dynamic_states] = {};

        uint32_t m_num_shader_stages = 0;
        static constexpr uint32_t s_max_shader_stages = static_cast<uint32_t>(ShaderStage::Frag) + 1;
        vk::PipelineShaderStageCreateInfo m_shader_stages[s_max_shader_stages] = {};

        vk::PipelineVertexInputStateCreateInfo m_vertex_input_state = {};
        vk::PipelineInputAssemblyStateCreateInfo m_input_assembly_state = {};
        vk::PipelineRasterizationStateCreateInfo m_rasterization_state = {};
        vk::PipelineMultisampleStateCreateInfo m_multisample_state = {};
        vk::PipelineDepthStencilStateCreateInfo m_depth_stencil_state = {};
        vk::PipelineTessellationStateCreateInfo m_tessellation_state = {};

        uint32_t m_num_color_attachments = 0;
        vk::PipelineColorBlendAttachmentState m_color_blend_attachments[s_max_color_attachments] = {};
        vk::Format m_color_attachment_formats[s_max_color_attachments] = {};

        vk::Format m_depth_attachment_format = vk::Format::eUndefined;
        vk::Format m_stencil_attachment_format = vk::Format::eUndefined;

        static uint32_t s_num_pipelines_created;
    };

    struct compute_pipeline_state final
    {
        compute_pipeline_desc desc;

        // non-owning
        vk::DescriptorSetLayout last_descriptor_set_layout = VK_NULL_HANDLE;

        vk::PipelineLayout pipeline_layout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;

        void* spec_constant_data_storage = nullptr;
    };

    struct ray_tracing_pipeline_state final
    {
        ray_tracing_pipeline_desc desc;

        // non-owning
        vk::DescriptorSetLayout last_descriptor_set_layout = VK_NULL_HANDLE;

        vk::ShaderStageFlags shader_stage_flags = {};
        vk::PipelineLayout pipeline_layout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;

        void* spec_constant_data_storage = nullptr;

        holder<buffer_handle> shader_binding_table = {};

        vk::StridedDeviceAddressRegionKHR sbt_entry_raygen = {};
        vk::StridedDeviceAddressRegionKHR sbt_entry_miss = {};
        vk::StridedDeviceAddressRegionKHR sbt_entry_hit = {};
        vk::StridedDeviceAddressRegionKHR sbt_entry_callable = {};
    };

    struct shader_module_state
    {
        vk::ShaderModule sm = VK_NULL_HANDLE;
        uint32_t push_constants_size = 0;
    };

    struct acceleration_structure
    {
        bool is_tlas = false;
        vk::AccelerationStructureBuildRangeInfoKHR build_range_info = {};
        vk::AccelerationStructureKHR vk_handle = VK_NULL_HANDLE;
        uint64_t device_address = 0;
        holder<buffer_handle> buffer = {};
        holder<buffer_handle> scratch_buffer = {}; // store only for TLAS
    };
}
