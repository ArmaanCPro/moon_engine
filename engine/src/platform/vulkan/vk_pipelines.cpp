#include "moonpch.h"
#include "vk_pipelines.h"

#include "utils/vk_utils.h"

namespace moon::vulkan
{
    uint32_t vk_pipeline_builder::s_num_pipelines_created = 0;

    vk_pipeline_builder::vk_pipeline_builder()
        :
        m_input_assembly_state({}, vk::PrimitiveTopology::eTriangleList, vk::False)
        , m_rasterization_state({}, vk::False, vk::False, vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, vk::False, 0.0f, 0.0f, 0.0f, 1.0f)
        , m_depth_stencil_state({}, vk::False, vk::False, vk::CompareOp::eLess, vk::False, vk::False,
            {
                vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eNever, 0, 0, 0
            }, {
                vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eNever, 0, 0, 0
            }, 0.0f, 1.0f)
    {}

    vk_pipeline_builder& vk_pipeline_builder::dynamic_state(vk::DynamicState state)
    {
        MOON_CORE_ASSERT(m_num_dynamic_states < s_max_dynamic_states);
        m_dynamic_states[m_num_dynamic_states++] = state;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::primitive_topology(vk::PrimitiveTopology topology)
    {
        m_input_assembly_state.topology = topology;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::rasterization_samples(vk::SampleCountFlagBits samples,
        float min_sample_shading)
    {
        m_multisample_state.rasterizationSamples = samples;
        m_multisample_state.sampleShadingEnable = min_sample_shading > 0.0f ? vk::True : vk::False;
        m_multisample_state.minSampleShading = min_sample_shading;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::shader_stage(vk::PipelineShaderStageCreateInfo stage)
    {
        if (stage.module != VK_NULL_HANDLE)
        {
            MOON_CORE_ASSERT(m_num_shader_stages < s_max_shader_stages);
            m_shader_stages[m_num_shader_stages++] = stage;
        }
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::stencil_state_ops(vk::StencilFaceFlags face_mask, vk::StencilOp failOp,
        vk::StencilOp passOp, vk::StencilOp depthFailOp, vk::CompareOp compareOp)
    {
        m_depth_stencil_state.stencilTestEnable = m_depth_stencil_state.stencilTestEnable == vk::True || failOp != vk::StencilOp::eKeep ||
            passOp != vk::StencilOp::eKeep || depthFailOp != vk::StencilOp::eKeep || compareOp != vk::CompareOp::eAlways
            ? vk::True : vk::False;

        if (face_mask & vk::StencilFaceFlagBits::eFront)
        {
            vk::StencilOpState& s = m_depth_stencil_state.front;
            s.failOp = failOp;
            s.passOp = passOp;
            s.depthFailOp = depthFailOp;
            s.compareOp = compareOp;
        }

        if (face_mask & vk::StencilFaceFlagBits::eBack)
        {
            vk::StencilOpState& s = m_depth_stencil_state.back;
            s.failOp = failOp;
            s.passOp = passOp;
            s.depthFailOp = depthFailOp;
            s.compareOp = compareOp;
        }
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::stencil_masks(vk::StencilFaceFlags face_mask, uint32_t compare_mask,
        uint32_t write_mask, uint32_t reference)
    {
        if (face_mask & vk::StencilFaceFlagBits::eFront)
        {
            m_depth_stencil_state.front.compareMask = compare_mask;
            m_depth_stencil_state.front.writeMask = write_mask;
            m_depth_stencil_state.front.reference = reference;
        }
        if (face_mask & vk::StencilFaceFlagBits::eBack)
        {
            m_depth_stencil_state.back.compareMask = compare_mask;
            m_depth_stencil_state.back.writeMask = write_mask;
            m_depth_stencil_state.back.reference = reference;
        }
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::cull_mode(vk::CullModeFlags mode)
    {
        m_rasterization_state.cullMode = mode;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::front_face(vk::FrontFace mode)
    {
        m_rasterization_state.frontFace = mode;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::polygon_mode(vk::PolygonMode mode)
    {
        m_rasterization_state.polygonMode = mode;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::vertex_input_state(const vk::PipelineVertexInputStateCreateInfo& state)
    {
        m_vertex_input_state = state;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::color_attachments(const vk::PipelineColorBlendAttachmentState* states,
        const vk::Format* formats, uint32_t num_color_attachments)
    {
        MOON_CORE_ASSERT(states);
        MOON_CORE_ASSERT(formats);
        MOON_CORE_ASSERT(num_color_attachments <= s_max_color_attachments);

        for (uint32_t i = 0; i != num_color_attachments; ++i)
        {
            m_color_blend_attachments[i] = states[i];
            m_color_attachment_formats[i] = formats[i];
        }
        m_num_color_attachments = num_color_attachments;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::depth_attachment_format(vk::Format format)
    {
        m_depth_attachment_format = format;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::stencil_attachment_format(vk::Format format)
    {
        m_stencil_attachment_format = format;
        return *this;
    }

    vk_pipeline_builder& vk_pipeline_builder::patch_control_points(uint32_t num_points)
    {
        m_tessellation_state.patchControlPoints = num_points;
        return *this;
    }

    vk::Pipeline vk_pipeline_builder::build(vk::Device device, vk::PipelineCache pipeline_cache,
        vk::PipelineLayout pipeline_layout, const char* debug_name) noexcept
    {
        const vk::PipelineDynamicStateCreateInfo dynamic_state = { {}, m_num_dynamic_states, m_dynamic_states };

        // viewport and scissor can be null if viewport state is dynamic
        constexpr vk::PipelineViewportStateCreateInfo viewport_state = { {}, 1, nullptr, 1, nullptr };
        const vk::PipelineColorBlendStateCreateInfo color_blend_state = {
            {}, false, vk::LogicOp::eCopy, m_num_color_attachments, m_color_blend_attachments
        };

        const vk::PipelineRenderingCreateInfo rendering_info = {
            {}, m_num_color_attachments, m_color_attachment_formats, m_depth_attachment_format, m_stencil_attachment_format
        };

        const vk::GraphicsPipelineCreateInfo pipeline_ci = {
            {}, m_num_shader_stages, m_shader_stages, &m_vertex_input_state, &m_input_assembly_state,
            &m_tessellation_state, &viewport_state, &m_rasterization_state, &m_multisample_state, &m_depth_stencil_state,
            &color_blend_state, &dynamic_state, pipeline_layout, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, -1,
            &rendering_info
        };

        const auto result = device.createGraphicsPipeline(pipeline_cache, pipeline_ci, nullptr);

        VK_CHECK(result.result);

        s_num_pipelines_created++;

        VK_CHECK(utils::set_debug_object_name(device, result.value.objectType, std::bit_cast<uint64_t>(result.value.operator VkPipeline()), debug_name));

        return result.value;
    }
}
