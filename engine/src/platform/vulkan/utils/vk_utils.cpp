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

    constexpr vk::Filter sampler_filter_to_vk_filter(SamplerFilter filter)
    {
        switch (filter)
        {
        case SamplerFilter::Nearest: return vk::Filter::eNearest;
        case SamplerFilter::Linear: return vk::Filter::eLinear;
        }
        MOON_CORE_ASSERT(false, "Unknown sampler filter! {}", (uint8_t)filter);
        return vk::Filter::eNearest;
    }

    constexpr vk::SamplerMipmapMode sampler_mipmap_to_vk_sampler_mipmap_mode(SamplerMip filter)
    {
        switch (filter)
        {
        case SamplerMip::Disabled:
        case SamplerMip::Nearest: return vk::SamplerMipmapMode::eNearest;
        case SamplerMip::Linear: return vk::SamplerMipmapMode::eLinear;
        }
        MOON_CORE_ASSERT(false, "Unknown sampler mip filter! {}", (uint8_t)filter);
        return vk::SamplerMipmapMode::eNearest;
    }

    constexpr vk::SamplerAddressMode sampler_wrap_mode_to_vk_sampler_address_mode(SamplerWrap mode)
    {
        switch (mode)
        {
        case SamplerWrap::Repeat: return vk::SamplerAddressMode::eRepeat;
        case SamplerWrap::Clamp: return vk::SamplerAddressMode::eClampToEdge;
        case SamplerWrap::MirrorRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
        }
        MOON_CORE_ASSERT(false, "Unknown sampler wrap mode! {}", (uint8_t)mode);
        return vk::SamplerAddressMode::eRepeat;
    }

    constexpr vk::CompareOp compare_op_to_vk_compare_op(CompareOp op)
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
        MOON_CORE_ASSERT(false, "Unknown compare op! {}", (uint8_t)op);
        return vk::CompareOp::eAlways;
    }

    constexpr stage_access get_pipeline_stage_access(vk::ImageLayout layout)
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

        MOON_CORE_ASSERT(false, "Unknown image layout transition! {}", (uint8_t)layout);
        return { vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite };
    }

    constexpr bool is_depth_or_stencil_vk_format(vk::Format format)
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
    }

    constexpr vk::IndexType index_format_to_vk_index_type(IndexFormat format)
    {
        switch (format)
        {
        case IndexFormat::Uint8:
            return vk::IndexType::eUint8;
        case IndexFormat::Uint16:
            return vk::IndexType::eUint16;
        case IndexFormat::Uint32:
            return vk::IndexType::eUint32;
        }
        MOON_CORE_ASSERT(false, "Unknown index format! {}", (uint8_t)format);
        return vk::IndexType::eUint32;
    }

    constexpr vk::PrimitiveTopology topology_to_vk_primitive_topology(Topology top)
    {
        switch (top)
        {
        case Topology::Point:
            return vk::PrimitiveTopology::ePointList;
        case Topology::Line:
            return vk::PrimitiveTopology::eLineList;
        case Topology::LineStrip:
            return vk::PrimitiveTopology::eLineStrip;
        case Topology::Triangle:
            return vk::PrimitiveTopology::eTriangleList;
        case Topology::TriangleStrip:
            return vk::PrimitiveTopology::eTriangleStrip;
        case Topology::Patch:
            return vk::PrimitiveTopology::ePatchList;
        }
        MOON_CORE_ASSERT(false, "Unknown topology! {}", (uint8_t)top);
        return vk::PrimitiveTopology::eTriangleList;
    }

    constexpr vk::AttachmentLoadOp load_op_to_vk_attachment_load_op(LoadOp op)
    {
        switch (op)
        {
        case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
        case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
        case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
        case LoadOp::None: return vk::AttachmentLoadOp::eNone;
        }
        MOON_CORE_ASSERT(false, "Unknown load op! {}", (uint8_t)op);
        return vk::AttachmentLoadOp::eDontCare;
    }

    constexpr vk::AttachmentStoreOp store_op_to_vk_attachment_store_op(StoreOp op)
    {
        switch (op)
        {
        case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
        case StoreOp::Store: return vk::AttachmentStoreOp::eStore;
        case StoreOp::MsaaResolve: return vk::AttachmentStoreOp::eDontCare; // we have to store data into a special "resolve" attachment
        case StoreOp::None: return vk::AttachmentStoreOp::eNone;
        }
        MOON_CORE_ASSERT(false, "Unknown store op! {}", (uint8_t)op);
        return vk::AttachmentStoreOp::eDontCare;
    }

    constexpr vk::ShaderStageFlagBits shader_stage_to_vk_shader_stage(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vert:
            return vk::ShaderStageFlagBits::eVertex;
        case ShaderStage::Tesc:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case ShaderStage::Tese:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case ShaderStage::Geom:
            return vk::ShaderStageFlagBits::eGeometry;
        case ShaderStage::Frag:
            return vk::ShaderStageFlagBits::eFragment;
        case ShaderStage::Comp:
            return vk::ShaderStageFlagBits::eCompute;
        case ShaderStage::Task:
            return vk::ShaderStageFlagBits::eTaskEXT;
        case ShaderStage::Mesh:
            return vk::ShaderStageFlagBits::eMeshEXT;
        case ShaderStage::RayGen:
            return vk::ShaderStageFlagBits::eRaygenKHR;
        case ShaderStage::AnyHit:
            return vk::ShaderStageFlagBits::eAnyHitKHR;
        case ShaderStage::ClosestHit:
            return vk::ShaderStageFlagBits::eClosestHitKHR;
        case ShaderStage::Miss:
            return vk::ShaderStageFlagBits::eMissKHR;
        case ShaderStage::Intersection:
            return vk::ShaderStageFlagBits::eIntersectionKHR;
        case ShaderStage::Callable:
            return vk::ShaderStageFlagBits::eCallableKHR;
        }
        MOON_CORE_ASSERT(false, "Unknown shader stage! {}", (uint8_t)stage);
        return vk::ShaderStageFlagBits::eVertex;
    }

    constexpr vk::MemoryPropertyFlags storage_type_to_vk_memory_property_flags(StorageType type)
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
            MOON_CORE_ASSERT(false, "Unknown storage type! {}", (uint8_t)type);
            break;
        }
        return flags;
    }

    constexpr vk::BuildAccelerationStructureFlagsKHR build_flags_to_vk_build_acceleration_structure_flags(uint8_t build_flags)
    {
        vk::BuildAccelerationStructureFlagsKHR flags = 0;
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

    constexpr vk::PolygonMode polygon_mode_to_vk_polygon_mode(PolygonMode mode)
    {
        switch (mode)
        {
        case PolygonMode::Fill: return vk::PolygonMode::eFill;
        case PolygonMode::Line: return vk::PolygonMode::eLine;
        }
        MOON_CORE_ASSERT(false, "Unknown polygon mode! {}", (uint8_t)mode);
        return vk::PolygonMode::eFill;
    }

    constexpr vk::BlendFactor blend_factor_to_vk_blend_factor(BlendFactor factor)
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
        MOON_CORE_ASSERT(false, "Unknown blend factor! {}", static_cast<uint8_t>(factor));
        return vk::BlendFactor::eOne;
    }

    constexpr vk::BlendOp blend_op_to_vk_blend_op(BlendOp op)
    {
        switch (op)
        {
            case BlendOp::Add: return vk::BlendOp::eAdd;
            case BlendOp::Subtract: return vk::BlendOp::eSubtract;
            case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
            case BlendOp::Min: return vk::BlendOp::eMin;
            case BlendOp::Max: return vk::BlendOp::eMax;
        }
        MOON_CORE_ASSERT(false, "Unknown blend op! {}", static_cast<uint8_t>(op));
        return vk::BlendOp::eAdd;
    }

    constexpr vk::CullModeFlags cull_mode_to_vk_cull_mode(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::None: return vk::CullModeFlagBits::eNone;
            case CullMode::Front: return vk::CullModeFlagBits::eFront;
            case CullMode::Back: return vk::CullModeFlagBits::eBack;
        }
        MOON_CORE_ASSERT(false, "Unknown cull mode! {}", static_cast<uint8_t>(mode));
        return vk::CullModeFlagBits::eNone;
    }

    constexpr vk::FrontFace winding_mode_vk_front_face(WindingMode mode)
    {
        switch (mode)
        {
        case WindingMode::CCW:
            return vk::FrontFace::eCounterClockwise;
        case WindingMode::CW:
            return vk::FrontFace::eClockwise;
        }
        MOON_CORE_ASSERT(false, "Unknown winding mode! {}", static_cast<uint8_t>(mode));
        return vk::FrontFace::eCounterClockwise;
    }

    constexpr vk::StencilOp stencil_op_to_vk_stencil_op(StencilOp op)
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
        MOON_CORE_ASSERT(false, "Unknown stencil op! {}", static_cast<uint8_t>(op));
        return vk::StencilOp::eKeep;
    }

    constexpr vk::Format vertex_format_to_vk_format(VertexFormat format)
    {
        switch (format)
        {
        case VertexFormat::Invalid:
            MOON_CORE_ASSERT(false, "Invalid vertex format!");
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
            // Normalized variants
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
        MOON_CORE_ASSERT(false, "Unknown vertex format! {}", static_cast<uint8_t>(format));
        return vk::Format::eUndefined;
    }

    constexpr uint32_t get_bytes_per_pixel(vk::Format format)
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
        MOON_CORE_ASSERT(false, "Unknown vertex format! {}", static_cast<uint8_t>(format));
        return 0;
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
            MOON_CORE_ASSERT(anisotropy_supported, "Anisotropic filtering is not supported on this device!");
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
            MOON_CORE_ASSERT(false, "Color attachments cannot have depth/stencil format!");
            return;
        }
        MOON_CORE_ASSERT(color_tex->m_format != vk::Format::eUndefined, "Invalid color attachment format!");
        color_tex->transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal,
            { vk::ImageAspectFlagBits::eColor, 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers });
    }
}
