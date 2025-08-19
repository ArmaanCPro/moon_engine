#pragma once

#include "vulkan/vk.h"

#include "core/log.h"
#include "renderer/render_types.h"
#include "vulkan/vk_render_types.h"

namespace moon::vulkan
{
    struct vulkan_image;
}

namespace moon::vulkan::utils
{
    [[nodiscard]] vk::Result set_debug_object_name(vk::Device device, vk::ObjectType type, uint64_t object, const char* name);

    [[nodiscard]] vk::Semaphore create_semaphore(vk::Device device, const char* debug_name = nullptr);

    [[nodiscard]] vk::Semaphore create_timeline_semaphore(vk::Device device, uint64_t initial_value, const char* debug_name = nullptr);

    [[nodiscard]] vk::Fence create_fence(vk::Device device, const char* debug_name = nullptr);

    [[nodiscard]] uint32_t find_queue_family_index(vk::PhysicalDevice physical_device, vk::QueueFlags flags);


    // conversion helper functions
    // ---------------------------

    [[nodiscard]] constexpr vk::Filter sampler_filter_to_vk_filter(SamplerFilter filter)
    {
        switch (filter)
        {
        case SamplerFilter::Nearest: return vk::Filter::eNearest;
        case SamplerFilter::Linear: return vk::Filter::eLinear;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown sampler filter! {}", (uint8_t)filter);
        return vk::Filter::eNearest;
    };

    [[nodiscard]] constexpr vk::SamplerMipmapMode sampler_mipmap_to_vk_sampler_mipmap_mode(SamplerMip filter)
    {
        switch (filter)
        {
        case SamplerMip::Disabled:
        case SamplerMip::Nearest: return vk::SamplerMipmapMode::eNearest;
        case SamplerMip::Linear: return vk::SamplerMipmapMode::eLinear;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown sampler mip filter! {}", (uint8_t)filter);
        return vk::SamplerMipmapMode::eNearest;
    };

    [[nodiscard]] constexpr vk::SamplerAddressMode sampler_wrap_mode_to_vk_sampler_address_mode(SamplerWrap mode)
    {
        switch (mode)
        {
        case SamplerWrap::Repeat: return vk::SamplerAddressMode::eRepeat;
        case SamplerWrap::Clamp: return vk::SamplerAddressMode::eClampToEdge;
        case SamplerWrap::MirrorRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown sampler wrap mode! {}", (uint8_t)mode);
        return vk::SamplerAddressMode::eRepeat;
    };

    [[nodiscard]] constexpr vk::CompareOp compare_op_to_vk_compare_op(CompareOp op)
    {
        switch (op)
        {
        case CompareOp::Never: return vk::CompareOp::eNever;
        case CompareOp::Less: return vk::CompareOp::eLess;
        case CompareOp::Equal: return vk::CompareOp::eEqual;
        case CompareOp::LessEqual: return vk::CompareOp::eLessOrEqual;
        case CompareOp::Greater: return vk::CompareOp::eGreater;
        case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
        case CompareOp::GreaterEqual: return vk::CompareOp::eGreaterOrEqual;
        case CompareOp::AlwaysPass: return vk::CompareOp::eAlways;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown compare op! {}", (uint8_t)op);
        return vk::CompareOp::eAlways;
    };

    [[nodiscard]] vk::SamplerCreateInfo sampler_state_desc_to_vk_sampler_create_info(const sampler_state_desc& desc,
                                                                                     const vk::PhysicalDeviceLimits& limits);

    [[nodiscard]] vk::SpecializationInfo get_pipeline_shader_stage_specialization_info(specialization_constant_desc desc,
                                                                                       vk::SpecializationMapEntry* out_entries);

    struct stage_access
    {
        vk::PipelineStageFlags2 stage;
        vk::AccessFlags2 access;
    };

    [[nodiscard]] constexpr stage_access get_pipeline_stage_access(vk::ImageLayout layout)
    {
        switch (layout)
        {
        case vk::ImageLayout::eUndefined: return { vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone };
        case vk::ImageLayout::eColorAttachmentOptimal:
            return { vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite };
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return { vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                     vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite };
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return { vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader |
                     vk::PipelineStageFlagBits2::ePreRasterizationShaders, vk::AccessFlagBits2::eShaderRead };
        case vk::ImageLayout::eTransferSrcOptimal:
            return { vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead };
        case vk::ImageLayout::eTransferDstOptimal:
            return { vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite };
        case vk::ImageLayout::eGeneral:
            return { vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eTransfer,
                     vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite };
        case vk::ImageLayout::ePresentSrcKHR:
            return { vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eComputeShader,
                     vk::AccessFlagBits2::eNone | vk::AccessFlagBits2::eShaderWrite };
        }

        MOON_CORE_ASSERT_MSG(false, "Unknown image layout transition! {}", (uint8_t)layout);
        return { vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite };
    };

    void image_memory_barrier_2(vk::CommandBuffer cmd, vk::Image image, stage_access src, stage_access dst,
                                vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageSubresourceRange subresource_range);

    void transition_to_color_attachment(vk::CommandBuffer cmd, vulkan_image* color_tex);

    [[nodiscard]] constexpr bool is_depth_or_stencil_vk_format(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
        case vk::Format::eS8Uint:
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return true;
        }
        return false;
    };

    [[nodiscard]] constexpr vk::IndexType index_format_to_vk_index_type(IndexFormat format)
    {
        switch (format)
        {
        case IndexFormat::Uint8: return vk::IndexType::eUint8;
        case IndexFormat::Uint16: return vk::IndexType::eUint16;
        case IndexFormat::Uint32: return vk::IndexType::eUint32;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown index format! {}", (uint8_t)format);
        return vk::IndexType::eUint32;
    };

    [[nodiscard]] constexpr vk::PrimitiveTopology topology_to_vk_primitive_topology(Topology top)
    {
        switch (top)
        {
        case Topology::Point: return vk::PrimitiveTopology::ePointList;
        case Topology::Line: return vk::PrimitiveTopology::eLineList;
        case Topology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
        case Topology::Triangle: return vk::PrimitiveTopology::eTriangleList;
        case Topology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
        case Topology::Patch: return vk::PrimitiveTopology::ePatchList;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown topology! {}", (uint8_t)top);
        return vk::PrimitiveTopology::eTriangleList;
    };

    [[nodiscard]] constexpr vk::AttachmentLoadOp load_op_to_vk_attachment_load_op(LoadOp op)
    {
        switch (op)
        {
        case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
        case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
        case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
        case LoadOp::None: return vk::AttachmentLoadOp::eNone;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown load op! {}", (uint8_t)op);
        return vk::AttachmentLoadOp::eDontCare;
    };

    [[nodiscard]] constexpr vk::AttachmentStoreOp store_op_to_vk_attachment_store_op(StoreOp op)
    {
        switch (op)
        {
        case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
        case StoreOp::Store: return vk::AttachmentStoreOp::eStore;
        case StoreOp::MsaaResolve: return vk::AttachmentStoreOp::eDontCare;
        case StoreOp::None: return vk::AttachmentStoreOp::eNone;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown store op! {}", (uint8_t)op);
        return vk::AttachmentStoreOp::eDontCare;
    };

    // ------------------------------
    // constexpr helpers from .cpp
    // ------------------------------

    [[nodiscard]] constexpr vk::ShaderStageFlagBits shader_stage_to_vk_shader_stage(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vert: return vk::ShaderStageFlagBits::eVertex;
        case ShaderStage::Tesc: return vk::ShaderStageFlagBits::eTessellationControl;
        case ShaderStage::Tese: return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case ShaderStage::Geom: return vk::ShaderStageFlagBits::eGeometry;
        case ShaderStage::Frag: return vk::ShaderStageFlagBits::eFragment;
        case ShaderStage::Comp: return vk::ShaderStageFlagBits::eCompute;
        case ShaderStage::Task: return vk::ShaderStageFlagBits::eTaskEXT;
        case ShaderStage::Mesh: return vk::ShaderStageFlagBits::eMeshEXT;
        case ShaderStage::RayGen: return vk::ShaderStageFlagBits::eRaygenKHR;
        case ShaderStage::AnyHit: return vk::ShaderStageFlagBits::eAnyHitKHR;
        case ShaderStage::ClosestHit: return vk::ShaderStageFlagBits::eClosestHitKHR;
        case ShaderStage::Miss: return vk::ShaderStageFlagBits::eMissKHR;
        case ShaderStage::Intersection: return vk::ShaderStageFlagBits::eIntersectionKHR;
        case ShaderStage::Callable: return vk::ShaderStageFlagBits::eCallableKHR;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown shader stage! {}", (uint8_t)stage);
        return vk::ShaderStageFlagBits::eVertex;
    }

    [[nodiscard]] constexpr vk::MemoryPropertyFlags storage_type_to_vk_memory_property_flags(StorageType type)
    {
        vk::MemoryPropertyFlags flags = {};
        switch (type)
        {
        case StorageType::Device:
            flags |= vk::MemoryPropertyFlagBits::eDeviceLocal;
            break;
        case StorageType::HostVisible:
            flags |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            break;
        case StorageType::MemoryLess:
            flags = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eLazilyAllocated;
            break;
        default:
            MOON_CORE_ASSERT_MSG(false, "Unknown storage type! {}", (uint8_t)type);
            break;
        }
        return flags;
    }

    [[nodiscard]] constexpr vk::BuildAccelerationStructureFlagsKHR build_flags_to_vk_build_acceleration_structure_flags(uint8_t build_flags)
    {
        vk::BuildAccelerationStructureFlagsKHR flags = {};
        if (build_flags & static_cast<uint8_t>(AccelStructBuildFlagBits::AllowUpdate))
            flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
        if (build_flags & static_cast<uint8_t>(AccelStructBuildFlagBits::AllowCompaction))
            flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
        if (build_flags & static_cast<uint8_t>(AccelStructBuildFlagBits::LowMemory))
            flags |= vk::BuildAccelerationStructureFlagBitsKHR::eLowMemory;
        if (build_flags & static_cast<uint8_t>(AccelStructBuildFlagBits::PreferFastTrace))
            flags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
        if (build_flags & static_cast<uint8_t>(AccelStructBuildFlagBits::PreferFastBuild))
            flags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
        return flags;
    }

    [[nodiscard]] constexpr vk::PolygonMode polygon_mode_to_vk_polygon_mode(PolygonMode mode)
    {
        switch (mode)
        {
        case PolygonMode::Fill: return vk::PolygonMode::eFill;
        case PolygonMode::Line: return vk::PolygonMode::eLine;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown polygon mode! {}", (uint8_t)mode);
        return vk::PolygonMode::eFill;
    }

    [[nodiscard]] constexpr vk::BlendFactor blend_factor_to_vk_blend_factor(BlendFactor factor)
    {
        switch (factor)
        {
        case BlendFactor::Zero: return vk::BlendFactor::eZero;
        case BlendFactor::One: return vk::BlendFactor::eOne;
        case BlendFactor::SrcColor: return vk::BlendFactor::eSrcColor;
        case BlendFactor::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
        case BlendFactor::DstColor: return vk::BlendFactor::eDstColor;
        case BlendFactor::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
        case BlendFactor::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
        case BlendFactor::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
        case BlendFactor::DstAlpha: return vk::BlendFactor::eDstAlpha;
        case BlendFactor::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
        case BlendFactor::BlendColor: return vk::BlendFactor::eConstantColor;
        case BlendFactor::OneMinusBlendColor: return vk::BlendFactor::eOneMinusConstantColor;
        case BlendFactor::BlendAlpha: return vk::BlendFactor::eConstantAlpha;
        case BlendFactor::OneMinusBlendAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
        case BlendFactor::SrcAlphaSaturated: return vk::BlendFactor::eSrcAlphaSaturate;
        case BlendFactor::Src1Color: return vk::BlendFactor::eSrc1Color;
        case BlendFactor::OneMinusSrc1Color: return vk::BlendFactor::eOneMinusSrc1Color;
        case BlendFactor::Src1Alpha: return vk::BlendFactor::eSrc1Alpha;
        case BlendFactor::OneMinusSrc1Alpha: return vk::BlendFactor::eOneMinusSrc1Alpha;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown blend factor! {}", static_cast<uint8_t>(factor));
        return vk::BlendFactor::eOne;
    }

    [[nodiscard]] constexpr vk::BlendOp blend_op_to_vk_blend_op(BlendOp op)
    {
        switch (op)
        {
            case BlendOp::Add: return vk::BlendOp::eAdd;
            case BlendOp::Subtract: return vk::BlendOp::eSubtract;
            case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
            case BlendOp::Min: return vk::BlendOp::eMin;
            case BlendOp::Max: return vk::BlendOp::eMax;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown blend op! {}", static_cast<uint8_t>(op));
        return vk::BlendOp::eAdd;
    }

    [[nodiscard]] constexpr vk::CullModeFlags cull_mode_to_vk_cull_mode(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::None: return vk::CullModeFlagBits::eNone;
            case CullMode::Front: return vk::CullModeFlagBits::eFront;
            case CullMode::Back: return vk::CullModeFlagBits::eBack;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown cull mode! {}", static_cast<uint8_t>(mode));
        return vk::CullModeFlagBits::eNone;
    }

    [[nodiscard]] constexpr vk::FrontFace winding_mode_vk_front_face(WindingMode mode)
    {
        switch (mode)
        {
        case WindingMode::CCW: return vk::FrontFace::eCounterClockwise;
        case WindingMode::CW: return vk::FrontFace::eClockwise;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown winding mode! {}", static_cast<uint8_t>(mode));
        return vk::FrontFace::eCounterClockwise;
    }

    [[nodiscard]] constexpr vk::StencilOp stencil_op_to_vk_stencil_op(StencilOp op)
    {
        switch (op)
        {
        case StencilOp::Keep: return vk::StencilOp::eKeep;
        case StencilOp::Zero: return vk::StencilOp::eZero;
        case StencilOp::Replace: return vk::StencilOp::eReplace;
        case StencilOp::IncrementClamp: return vk::StencilOp::eIncrementAndClamp;
        case StencilOp::DecrementClamp: return vk::StencilOp::eDecrementAndClamp;
        case StencilOp::Invert: return vk::StencilOp::eInvert;
        case StencilOp::IncrementWrap: return vk::StencilOp::eIncrementAndWrap;
        case StencilOp::DecrementWrap: return vk::StencilOp::eDecrementAndWrap;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown stencil op! {}", static_cast<uint8_t>(op));
        return vk::StencilOp::eKeep;
    }

    [[nodiscard]] constexpr vk::Format vertex_format_to_vk_format(VertexFormat format)
    {
        switch (format)
        {
        case VertexFormat::Invalid:
            MOON_CORE_ASSERT_MSG(false, "Invalid vertex format!");
            return vk::Format::eUndefined;
        case VertexFormat::Float1: return vk::Format::eR32Sfloat;
        case VertexFormat::Float2: return vk::Format::eR32G32Sfloat;
        case VertexFormat::Float3: return vk::Format::eR32G32B32Sfloat;
        case VertexFormat::Float4: return vk::Format::eR32G32B32A32Sfloat;
        case VertexFormat::Byte1: return vk::Format::eR8Sint;
        case VertexFormat::Byte2: return vk::Format::eR8G8Sint;
        case VertexFormat::Byte3: return vk::Format::eR8G8B8Sint;
        case VertexFormat::Byte4: return vk::Format::eR8G8B8A8Sint;
        case VertexFormat::UByte1: return vk::Format::eR8Uint;
        case VertexFormat::UByte2: return vk::Format::eR8G8Uint;
        case VertexFormat::UByte3: return vk::Format::eR8G8B8Uint;
        case VertexFormat::UByte4: return vk::Format::eR8G8B8A8Uint;
        case VertexFormat::Short1: return vk::Format::eR16Sint;
        case VertexFormat::Short2: return vk::Format::eR16G16Sint;
        case VertexFormat::Short3: return vk::Format::eR16G16B16Sint;
        case VertexFormat::Short4: return vk::Format::eR16G16B16A16Sint;
        case VertexFormat::UShort1: return vk::Format::eR16Uint;
        case VertexFormat::UShort2: return vk::Format::eR16G16Uint;
        case VertexFormat::UShort3: return vk::Format::eR16G16B16Uint;
        case VertexFormat::UShort4: return vk::Format::eR16G16B16A16Uint;
        case VertexFormat::Byte2Norm: return vk::Format::eR8G8Snorm;
        case VertexFormat::Byte4Norm: return vk::Format::eR8G8B8A8Snorm;
        case VertexFormat::UByte2Norm: return vk::Format::eR8G8Unorm;
        case VertexFormat::UByte4Norm: return vk::Format::eR8G8B8A8Unorm;
        case VertexFormat::Short2Norm: return vk::Format::eR16G16Snorm;
        case VertexFormat::Short4Norm: return vk::Format::eR16G16B16A16Snorm;
        case VertexFormat::UShort2Norm: return vk::Format::eR16G16Unorm;
        case VertexFormat::UShort4Norm: return vk::Format::eR16G16B16A16Unorm;
        case VertexFormat::Int1: return vk::Format::eR32Sint;
        case VertexFormat::Int2: return vk::Format::eR32G32Sint;
        case VertexFormat::Int3: return vk::Format::eR32G32B32Sint;
        case VertexFormat::Int4: return vk::Format::eR32G32B32A32Sint;
        case VertexFormat::UInt1: return vk::Format::eR32Uint;
        case VertexFormat::UInt2: return vk::Format::eR32G32Uint;
        case VertexFormat::UInt3: return vk::Format::eR32G32B32Uint;
        case VertexFormat::UInt4: return vk::Format::eR32G32B32A32Uint;
        case VertexFormat::HalfFloat1: return vk::Format::eR16Sfloat;
        case VertexFormat::HalfFloat2: return vk::Format::eR16G16Sfloat;
        case VertexFormat::HalfFloat3: return vk::Format::eR16G16B16Sfloat;
        case VertexFormat::HalfFloat4: return vk::Format::eR16G16B16A16Sfloat;
        case VertexFormat::Int_2_10_10_10_Rev: return vk::Format::eA2B10G10R10SnormPack32;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown vertex format! {}", static_cast<uint8_t>(format));
        return vk::Format::eUndefined;
    }

    [[nodiscard]] constexpr uint32_t get_bytes_per_pixel(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eR8Unorm: return 1;
        case vk::Format::eR16Sfloat: return 2;
        case vk::Format::eR8G8B8Unorm:
        case vk::Format::eB8G8R8Unorm: return 3;
        case vk::Format::eR8G8B8A8Unorm:
        case vk::Format::eB8G8R8A8Unorm:
        case vk::Format::eR8G8B8A8Srgb:
        case vk::Format::eR16G16Sfloat:
        case vk::Format::eR32Sfloat:
        case vk::Format::eR32Uint: return 4;
        case vk::Format::eR16G16B16Sfloat: return 6;
        case vk::Format::eR16G16B16A16Sfloat:
        case vk::Format::eR32G32Sfloat:
        case vk::Format::eR32G32Uint: return 8;
        case vk::Format::eR32G32B32Sfloat: return 12;
        case vk::Format::eR32G32B32A32Sfloat: return 16;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown vertex format! {}", static_cast<uint8_t>(format));
        return 0;
    }

    [[nodiscard]] constexpr vk::SampleCountFlagBits get_vulkan_samples_count_flags(uint32_t samples,
                                                                                   vk::SampleCountFlagBits max_samples_mask)
    {
        if (samples <= 1 || vk::SampleCountFlagBits::e2 > max_samples_mask)
            return vk::SampleCountFlagBits::e1;
        if (samples <= 2 || vk::SampleCountFlagBits::e4 > max_samples_mask)
            return vk::SampleCountFlagBits::e2;
        if (samples <= 4 || vk::SampleCountFlagBits::e8 > max_samples_mask)
            return vk::SampleCountFlagBits::e4;
        if (samples <= 8 || vk::SampleCountFlagBits::e16 > max_samples_mask)
            return vk::SampleCountFlagBits::e8;
        if (samples <= 16 || vk::SampleCountFlagBits::e32 > max_samples_mask)
            return vk::SampleCountFlagBits::e16;
        if (samples <= 32 || vk::SampleCountFlagBits::e64 > max_samples_mask)
            return vk::SampleCountFlagBits::e32;
        return vk::SampleCountFlagBits::e64;
    }

    [[nodiscard]] constexpr vk::PipelineShaderStageCreateInfo get_pipeline_shader_stage_create_info(
        vk::ShaderStageFlagBits stage, vk::ShaderModule shader_module,
        const char* entry_point = "main", const vk::SpecializationInfo* specialization_info = nullptr)
    {
        return vk::PipelineShaderStageCreateInfo{
            {}, stage, shader_module, entry_point ? entry_point : "main", specialization_info
        };
    }

    [[nodiscard]] constexpr uint32_t get_aligned_size(uint32_t value, uint32_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    [[nodiscard]] constexpr vk::DescriptorSetLayoutBinding get_dsl_binding(uint32_t binding, vk::DescriptorType type, uint32_t count,
                                                                           vk::ShaderStageFlags stage_flags, const vk::Sampler* immutable_samplers = nullptr)
    {
        return vk::DescriptorSetLayoutBinding{
            binding, type, count, stage_flags, immutable_samplers
        };
    }

    [[nodiscard]] constexpr vk::Format format_to_vk_format(Format format)
    {
        switch (format)
        {
        case Format::Invalid: return vk::Format::eUndefined;
        case Format::R_UN8: return vk::Format::eR8Unorm;
        case Format::R_UN16: return vk::Format::eR16Unorm;
        case Format::R_F16: return vk::Format::eR16Sfloat;
        case Format::R_UI16: return vk::Format::eR16Uint;
        case Format::R_UI32: return vk::Format::eR32Uint;
        case Format::RG_UN8: return vk::Format::eR8G8Unorm;
        case Format::RG_UI16: return vk::Format::eR16G16Uint;
        case Format::RG_UI32: return vk::Format::eR32G32Uint;
        case Format::RG_UN16: return vk::Format::eR16G16Unorm;
        case Format::BGRA_UN8: return vk::Format::eB8G8R8A8Unorm;
        case Format::RGBA_UN8: return vk::Format::eR8G8B8A8Unorm;
        case Format::RGBA_SRGB8: return vk::Format::eR8G8B8A8Srgb;
        case Format::BGRA_SRGB8: return vk::Format::eB8G8R8A8Srgb;
        case Format::RG_F16: return vk::Format::eR16G16Sfloat;
        case Format::RG_F32: return vk::Format::eR32G32Sfloat;
        case Format::R_F32: return vk::Format::eR32Sfloat;
        case Format::RGBA_F16: return vk::Format::eR16G16B16A16Sfloat;
        case Format::RGBA_UI32: return vk::Format::eR32G32B32A32Uint;
        case Format::RGBA_F32: return vk::Format::eR32G32B32A32Sfloat;
        case Format::ETC2_RGB8: return vk::Format::eEtc2R8G8B8UnormBlock;
        case Format::ETC2_SRGB8: return vk::Format::eEtc2R8G8B8SrgbBlock;
        case Format::BC7_RGBA: return vk::Format::eBc7UnormBlock;
        case Format::Z_UN16: return vk::Format::eD16Unorm;
        case Format::Z_UN24: return vk::Format::eD24UnormS8Uint;
        case Format::Z_F32: return vk::Format::eD32Sfloat;
        case Format::Z_UN24_S_UI8: return vk::Format::eD24UnormS8Uint;
        case Format::Z_F32_S_UI8: return vk::Format::eD32SfloatS8Uint;
        case Format::YUV_NV12: return vk::Format::eG8B8R82Plane420Unorm;
        case Format::YUV_420p: return vk::Format::eG8B8R83Plane420Unorm;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown format! {}", static_cast<uint8_t>(format));
        return vk::Format::eUndefined;
    }

    [[nodiscard]] constexpr Format vk_format_to_format(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eUndefined: return Format::Invalid;
        case vk::Format::eR8Unorm: return Format::R_UN8;
        case vk::Format::eR16Unorm: return Format::R_UN16;
        case vk::Format::eR16Sfloat: return Format::R_F16;
        case vk::Format::eR16Uint: return Format::R_UI16;
        case vk::Format::eR8G8Unorm: return Format::RG_UN8;
        case vk::Format::eB8G8R8A8Unorm: return Format::BGRA_UN8;
        case vk::Format::eR8G8B8A8Unorm: return Format::RGBA_UN8;
        case vk::Format::eR8G8B8A8Srgb: return Format::RGBA_SRGB8;
        case vk::Format::eB8G8R8A8Srgb: return Format::BGRA_SRGB8;
        case vk::Format::eR16G16Unorm: return Format::RG_UN16;
        case vk::Format::eR16G16Sfloat: return Format::RG_F16;
        case vk::Format::eR32G32Sfloat: return Format::RG_F32;
        case vk::Format::eR16G16Uint: return Format::RG_UI16;
        case vk::Format::eR32Sfloat: return Format::R_F32;
        case vk::Format::eR16G16B16A16Sfloat: return Format::RGBA_F16;
        case vk::Format::eR32G32B32A32Uint: return Format::RGBA_UI32;
        case vk::Format::eR32G32B32A32Sfloat: return Format::RGBA_F32;
        case vk::Format::eEtc2R8G8B8UnormBlock: return Format::ETC2_RGB8;
        case vk::Format::eEtc2R8G8B8SrgbBlock: return Format::ETC2_SRGB8;
        case vk::Format::eD16Unorm: return Format::Z_UN16;
        case vk::Format::eBc7UnormBlock: return Format::BC7_RGBA;
        case vk::Format::eX8D24UnormPack32: return Format::Z_UN24;
        case vk::Format::eD24UnormS8Uint: return Format::Z_UN24_S_UI8;
        case vk::Format::eD32Sfloat: return Format::Z_F32;
        case vk::Format::eD32SfloatS8Uint: return Format::Z_F32_S_UI8;
        case vk::Format::eG8B8R82Plane420Unorm: return Format::YUV_NV12;
        case vk::Format::eG8B8R83Plane420Unorm: return Format::YUV_420p;
        }
        MOON_CORE_ASSERT_MSG(false, "Unknown format! {}", static_cast<uint8_t>(format));
        return Format::Invalid;
    }

    [[nodiscard]] constexpr bool is_depth_format(vk::Format format)
    {
        return (format == vk::Format::eS8Uint) || (format == vk::Format::eX8D24UnormPack32) || (format == vk::Format::eD32Sfloat)
               || (format == vk::Format::eD16UnormS8Uint) || (format == vk::Format::eD24UnormS8Uint)
               || (format == vk::Format::eD32SfloatS8Uint);
    }

    [[nodiscard]] constexpr bool is_stencil_format(vk::Format format)
    {
        return (format == vk::Format::eS8Uint) || (format == vk::Format::eD16UnormS8Uint) || (format == vk::Format::eD24UnormS8Uint)
               || (format == vk::Format::eD32SfloatS8Uint);
    }
}
