#pragma once

#include "handle.h"

#include <array>
#include <cstdint>

#include <limits>

namespace moon
{
    static constexpr int s_max_color_attachments = 8;
    static constexpr int s_max_mip_levels = 16;

    enum class IndexFormat : uint8_t
    {
        Uint8,
        Uint16,
        Uint32
    };

    enum class Topology : uint8_t
    {
        Point,
        Line,
        LineStrip,
        Triangle,
        TriangleStrip,
        Patch
    };

    enum class ColorSpace : uint8_t
    {
        SRGB_Linear,
        SRGB_Nonlinear
    };

    enum class TextureType : uint8_t
    {
        e2D,
        e3D,
        eCube,
    };

    enum class SamplerFilter : uint8_t
    {
        Nearest = 0,
        Linear
    };
    enum class SamplerMip : uint8_t
    {
        Disabled = 0,
        Nearest,
        Linear
    };
    enum class SamplerWrap : uint8_t
    {
        Repeat = 0,
        Clamp,
        MirrorRepeat
    };

    enum class HWDeviceType
    {
        Discrete = 1,
        External = 2,
        Integrated = 3,
        Software = 4
    };

    struct HWDeviceDesc
    {
        static constexpr int s_max_physical_device_name_size = 256;
        uintptr_t guid = 0;
        HWDeviceType type = HWDeviceType::Discrete;
        std::array<char, s_max_physical_device_name_size> name = {};
    };

    enum class StorageType
    {
        Device,
        HostVisible,
        MemoryLess
    };

    enum class CullMode : uint8_t
    {
        None,
        Front,
        Back
    };
    enum class WindingMode : uint8_t
    {
        CCW, CW
    };

    struct result
    {
        enum class Code
        {
            Ok,
            ArgumentOutOfRange,
            RuntimeError
        };

        Code code = Code::Ok;
        const char* message = "";
        explicit result() = default;
        explicit result(Code code, const char* message = "") : code(code), message(message) {}

        bool is_ok() const { return code == Code::Ok; }

        static void set_result(result* out_result, Code code, const char* message = "") noexcept
        {
            if (out_result)
            {
                out_result->code = code;
                out_result->message = message;
            }
        }
        static void set_result(result* out_result, const result& source_result) noexcept
        {
            if (out_result)
            {
                *out_result = source_result;
            }
        }
    };

    struct scissor_rect
    {
        uint32_t x = 0, y = 0, width = 0, height = 0;
    };

    struct dimensions
    {
        uint32_t width = 1, height = 1, depth = 1;
        [[nodiscard]] inline dimensions divide1D(uint32_t v) const noexcept
        {
            return { .width = width / v, .height = height, .depth = depth };
        }
        [[nodiscard]] inline dimensions divide2D(uint32_t v) const noexcept
        {
            return { .width = width / v, .height = height / v, .depth = depth };
        }
        [[nodiscard]] inline dimensions divide3D(uint32_t v) const noexcept
        {
            return { .width = width / v, .height = height / v, .depth = depth / v };
        }
        inline bool operator==(const dimensions& other) const noexcept
        {
            return width == other.width && height == other.height && depth == other.depth;
        }
        inline bool operator!=(const dimensions& other) const noexcept
        {
            return !(*this == other);
        }
    };

    struct viewport
    {
        float x = 0.0f, y = 0.0f;
        float width = 1.0f, height = 1.0f;
        float min_depth = 0.0f, max_depth = 1.0f;
    };

    enum class CompareOp : uint8_t
    {
        Never = 0,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        AlwaysPass
    };

    enum class StencilOp : uint8_t
    {
        Keep = 0,
        Zero,
        Replace,
        IncrementClamp,
        DecrementClamp,
        Invert,
        IncrementWrap,
        DecrementWrap
    };

    enum class BlendOp : uint8_t
    {
        Add = 0,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class BlendFactor : uint8_t
    {
        Zero = 0,
        One,
        SrcColor,
        OneMinusSrcColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstColor,
        OneMinusDstColor,
        DstAlpha,
        OneMinusDstAlpha,
        SrcAlphaSaturated,
        BlendColor,
        OneMinusBlendColor,
        BlendAlpha,
        OneMinusBlendAlpha,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };

    struct sampler_state_desc
    {
        SamplerFilter min_filter = SamplerFilter::Linear;
        SamplerFilter mag_filter = SamplerFilter::Linear;
        SamplerMip mip_map = SamplerMip::Disabled;
        SamplerWrap wrap_u = SamplerWrap::Repeat;
        SamplerWrap wrap_v = SamplerWrap::Repeat;
        SamplerWrap wrap_w = SamplerWrap::Repeat;
        CompareOp depth_compare_op = CompareOp::LessEqual; // consider using GreaterEqual by default for modern inverse-z depth
        uint8_t mipLodMin = 0;
        uint8_t mipLodMax = s_max_mip_levels - 1;
        uint8_t max_anisotropic = 1;
        bool depth_compare_enabled = false;
        const char* debug_name = "";
    };

    struct stencil_state
    {
        StencilOp stencil_failure_op = StencilOp::Keep;
        StencilOp depth_failure_op = StencilOp::Keep;
        StencilOp depth_stencil_pass_op = StencilOp::Keep;
        CompareOp stencil_compare_op = CompareOp::AlwaysPass;
        uint32_t read_mask = std::numeric_limits<uint32_t>::max();
        uint32_t write_mask = std::numeric_limits<uint32_t>::max();
    };

    struct depth_state
    {
        CompareOp compare_op = CompareOp::AlwaysPass;
        bool depth_write_enabled = true;
    };

    enum class PolygonMode : uint8_t
    {
        Fill = 0,
        Line
    };

    enum class VertexFormat
    {
        Invalid = 0,

        Float1,
        Float2,
        Float3,
        Float4,

        Byte1,
        Byte2,
        Byte3,
        Byte4,

        UByte1,
        UByte2,
        UByte3,
        UByte4,

        Short1,
        Short2,
        Short3,
        Short4,

        UShort1,
        UShort2,
        UShort3,
        UShort4,

        Byte2Norm,
        Byte4Norm,

        UByte2Norm,
        UByte4Norm,

        Short2Norm,
        Short4Norm,

        UShort2Norm,
        UShort4Norm,

        Int1,
        Int2,
        Int3,
        Int4,

        UInt1,
        UInt2,
        UInt3,
        UInt4,

        HalfFloat1,
        HalfFloat2,
        HalfFloat3,
        HalfFloat4,

        Int_2_10_10_10_Rev
    };

    enum class Format : uint8_t
    {
        Invalid = 0,

        R_UN8,
        R_UI16,
        R_UI32,
        R_UN16,
        R_F16,
        R_F32,

        RG_UN8,
        RG_UI16,
        RG_UI32,
        RG_UN16,
        RG_F16,
        RG_F32,

        RGBA_UN8,
        RGBA_UI32,
        RGBA_F16,
        RGBA_F32,
        RGBA_SRGB8,

        BGRA_UN8,
        BGRA_SRGB8,

        ETC2_RGB8,
        ETC2_SRGB8,
        BC7_RGBA,

        Z_UN16,
        Z_UN24,
        Z_F32,
        Z_UN24_S_UI8,
        Z_F32_S_UI8,

        YUV_NV12,
        YUV_420p
    };

    enum class LoadOp : uint8_t
    {
        Invalid = 0,
        DontCare,
        Load,
        Clear,
        None
    };

    enum class StoreOp : uint8_t
    {
        DontCare = 0,
        Store,
        MsaaResolve,
        None
    };

    enum class ShaderStage : uint8_t
    {
        Vert,
        Tesc, // Tesselation Control
        Tese, // Tesselation Evaluation
        Geom,
        Frag,
        Comp,
        Task,
        Mesh,
        // Ray Tracing
        RayGen,
        AnyHit,
        ClosestHit,
        Miss,
        Intersection,
        Callable
    };

    union ClearColorValue
    {
        float float32[4];
        int32_t int32[4];
        uint32_t uint32[4];
    };

    struct vertex_input final
    {
        static constexpr auto s_vertex_attribute_max = 16u;
        static constexpr auto s_vertex_buffer_max = 16u;
        struct vertex_attribute final
        {
            uint32_t location = 0;
            uint32_t binding = 0;
            VertexFormat format = VertexFormat::Invalid;
            uintptr_t offset = 0;
        };
        std::array<vertex_attribute, s_vertex_attribute_max> attributes;

        struct vertex_input_binding final
        {
            uint32_t stride = 0;
        };
        std::array<vertex_input_binding, s_vertex_buffer_max> bindings;

        [[nodiscard]] uint32_t get_num_attributes() const noexcept
        {
            uint32_t n = 0;
            while (n < s_vertex_attribute_max && attributes[n].format != VertexFormat::Invalid)
                n++;
            return n;
        }
        [[nodiscard]] uint32_t get_num_input_bindings() const noexcept
        {
            uint32_t n = 0;
            while (n < s_vertex_buffer_max && bindings[n].stride != 0)
                n++;
            return n;
        }

        bool operator==(const vertex_input& other) const noexcept
        {
            return std::memcmp(this, &other, sizeof(vertex_input)) == 0;
        };
    };

    struct color_attachment
    {
        Format format = Format::Invalid;
        bool blend_enabled = false;
        BlendOp rgb_blend_op = BlendOp::Add;
        BlendOp alpha_blend_op = BlendOp::Add;
        BlendFactor src_rgb_blend_factor = BlendFactor::One;
        BlendFactor src_alpha_blend_factor = BlendFactor::One;
        BlendFactor dst_rgb_blend_factor = BlendFactor::Zero;
        BlendFactor dst_alpha_blend_factor = BlendFactor::Zero;
    };

    struct shader_module_desc
    {
        ShaderStage stage = ShaderStage::Vert;
        const char* data = nullptr;
        std::size_t data_size = 0; // if data_size is non-zero, interpret data is binary shader data
        const char* debug_name = "";

        shader_module_desc(const char* source, ShaderStage stage, const char* debug_name = "") :
            stage(stage), data(source), debug_name(debug_name)
        {}
        shader_module_desc(const void* data, std::size_t data_length, ShaderStage stage, const char* debug_name = "") :
            stage(stage), data(static_cast<const char*>(data)), data_size(data_length), debug_name(debug_name)
        {}
    };

    struct specialization_constant_entry
    {
        uint32_t constant_id = 0;
        uint32_t offset = 0; // offset within specialization_constant_desc::data
        std::size_t size = 0;
    };

    struct specialization_constant_desc
    {
        static constexpr auto s_specialization_constants_max = 16u;
        std::array<specialization_constant_entry, s_specialization_constants_max> entries;
        const void* data = nullptr;
        std::size_t data_size = 0;
        [[nodiscard]] uint32_t get_num_specialization_constants() const noexcept
        {
            uint32_t n = 0;
            while (n < s_specialization_constants_max && entries[n].size)
                n++;
            return n;
        }
    };

    struct render_pipeline_desc final
    {
        Topology topology = Topology::Triangle;

        vertex_input vertex_input;

        shader_module_handle sm_vert;
        shader_module_handle sm_tesc;
        shader_module_handle sm_tese;
        shader_module_handle sm_geom;
        shader_module_handle sm_task;
        shader_module_handle sm_mesh;
        shader_module_handle sm_frag;

        specialization_constant_desc spec_info = {};

        const char* entry_point_vert = "main";
        const char* entry_point_tesc = "main";
        const char* entry_point_tese = "main";
        const char* entry_point_geom = "main";
        const char* entry_point_task = "main";
        const char* entry_point_mesh = "main";
        const char* entry_point_frag = "main";

        std::array<color_attachment, s_max_color_attachments> color_attachments;

        Format depth_format = Format::Invalid;
        Format stencil_format = Format::Invalid;

        CullMode cull_mode = CullMode::None;
        WindingMode winding_mode = WindingMode::CCW;
        PolygonMode polygon_mode = PolygonMode::Fill;

        stencil_state back_face_stencil = {};
        stencil_state front_face_stencil = {};

        uint32_t samples_count = 1u;
        uint32_t patch_control_points = 0u;
        float min_sample_shading = 0.0f;

        const char* debug_name = "";

        [[nodiscard]] uint32_t get_num_color_attachments() const noexcept
        {
            uint32_t n = 0;
            while (n < s_max_color_attachments && color_attachments[n].format != Format::Invalid)
                n++;
            return n;
        }
    };

    struct compute_pipeline_desc final
    {
        shader_module_handle sm_comp;
        specialization_constant_desc spec_info = {};
        const char* entry_point_comp = "main";
        const char* debug_name = "";
    };

    struct ray_tracing_pipeline_desc final
    {
        shader_module_handle sm_raygen;
        shader_module_handle sm_any_hit;
        shader_module_handle sm_closest_hit;
        shader_module_handle sm_miss;
        shader_module_handle sm_intersection;
        shader_module_handle sm_callable;
        specialization_constant_desc spec_info = {};
        const char* entry_point = "main";
        const char* debug_name = "";
    };

    // still compatible with dynamic rendering api
    struct render_pass final
    {
        struct attachment_desc final
        {
            LoadOp load_op = LoadOp::Invalid;
            StoreOp store_op = StoreOp::Store;
            uint8_t layer = 0;
            uint8_t level = 0;
            float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            float clear_depth = 1.0f;
            uint8_t clear_stencil = 0;
        };

        std::array<attachment_desc, s_max_color_attachments> color;
        attachment_desc depth = { .load_op = LoadOp::DontCare, .store_op = StoreOp::DontCare };
        attachment_desc stencil = { .load_op = LoadOp::DontCare, .store_op = StoreOp::DontCare };

        [[nodiscard]] uint32_t get_num_color_attachments() const noexcept
        {
            uint32_t n = 0;
            while (n < s_max_color_attachments && color[n].load_op != LoadOp::Invalid)
                n++;
            return n;
        }
    };

    // still compatible with dynamic rendering
    struct framebuffer final
    {
        struct attachment_desc final
        {
            texture_handle texture;
            texture_handle resolve_texture;
        };

        std::array<attachment_desc, s_max_color_attachments> color;
        attachment_desc depth_stencil = {};

        const char* debug_name = "";

        [[nodiscard]] uint32_t get_num_color_attachments() const noexcept
        {
            uint32_t n = 0;
            while (n < s_max_color_attachments && color[n].texture)
                n++;
            return n;
        }
    };

    enum class BufferUsageBits : uint8_t
    {
        Index = 1 << 0,
        Vertex = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4,
        // ray tracing
        shader_binding_table = 1 << 5,
        accel_struct_build_input_read_only = 1 << 6,
        accel_struct_storage = 1 << 7
    };

    struct buffer_desc final
    {
        uint8_t usage = 0;
        StorageType storage_type = StorageType::HostVisible;
        std::size_t size = 0;
        const void* data = nullptr;
        const char* debug_name = "";
    };

    struct offset3D
    {
        int32_t x = 0, y = 0, z = 0;
    };

    struct texture_layers
    {
        uint32_t mip_level = 0;
        uint32_t layer = 0;
        uint32_t num_layers = 1;
    };

    struct texture_range_desc
    {
        offset3D offset = {};
        dimensions dimensions = {1, 1, 1};
        uint32_t layer = 0;
        uint32_t num_layers = 1;
        uint32_t mip_level = 0;
        uint32_t num_mip_levels = 1;
    };

    enum class TextureUsageBits : uint8_t
    {
        Sampled = 1 << 0,
        Storage = 1 << 1,
        Attachment = 1 << 2
    };

    enum class Swizzle : uint8_t
    {
        Default = 0,
        e0,
        e1,
        R,
        G,
        B,
        A
    };

    struct component_mapping
    {
        Swizzle r = Swizzle::Default;
        Swizzle g = Swizzle::Default;
        Swizzle b = Swizzle::Default;
        Swizzle a = Swizzle::Default;
        [[nodiscard]] bool identity() const noexcept
        {
            return r == Swizzle::Default && g == Swizzle::Default && b == Swizzle::Default && a == Swizzle::Default;
        }
    };

    struct texture_desc
    {
        TextureType type = TextureType::e2D;
        Format format = Format::Invalid;

        dimensions dimensions = {1, 1, 1};
        uint32_t num_layers = 1;
        uint32_t num_samples = 1;
        uint8_t usage = static_cast<uint8_t>(TextureUsageBits::Sampled);
        uint32_t num_mip_levels = 1;
        StorageType storage_type = StorageType::Device;
        component_mapping swizzle = {};
        const void* data = nullptr;
        uint32_t data_num_mip_levels = 1; // how many mips to upload
        bool generate_mipmaps = false; // gen mip_levels immediately, valid with only non-null data
        const char* debug_name = "";
    };

    struct texture_view_desc
    {
        TextureType type = TextureType::e2D;
        uint32_t layer = 0;
        uint32_t num_layers = 1;
        uint32_t mip_level = 0;
        uint32_t num_mip_levels = 1;
        component_mapping swizzle = {};
    };

    enum class AccelStructType : uint8_t
    {
        Invalid = 0,
        TLAS,
        BLAS
    };

    enum class AccelStructGeomType : uint8_t
    {
        Triangles = 0,
        AABBs,
        Instances
    };

    enum class AccelStructBuildFlagBits : uint8_t
    {
        AllowUpdate = 1 << 0,
        AllowCompaction = 1 << 1,
        PreferFastTrace = 1 << 2,
        PreferFastBuild = 1 << 3,
        LowMemory = 1 << 4,
    };

    enum class AccelStructGeometryFlagBits : uint8_t
    {
        Opaque = 1 << 0,
        NoDuplicateAnyHit = 1 << 1
    };

    enum class AccelStructInstanceFlagBits : uint8_t
    {
        TriangleFacingCullDisable = 1 << 0,
        TriangleFlipFacing = 1 << 1,
        ForceOpaque = 1 << 2,
        ForceNoOpaque = 1 << 3
    };

    struct accel_struct_build_range
    {
        uint32_t primive_count = 0;
        uint32_t primitive_offset = 0;
        uint32_t first_vertex = 0;
        uint32_t transform_offset = 0;
    };

    struct mat3x4
    {
        float matrix[3][4];
    };

    struct accel_struct_instance
    {
        mat3x4 transform;
        uint32_t instance_custom_index : 24 = 0;
        uint32_t mask : 8 = 0xff;
        uint32_t instance_shader_binding_table_record_offset : 24 = 0;
        uint32_t flags : 8 = static_cast<uint32_t>(AccelStructInstanceFlagBits::TriangleFacingCullDisable);
        uint64_t acceleration_structure_reference = 0;
    };

    struct accel_struct_desc
    {
        AccelStructType type = AccelStructType::Invalid;
        AccelStructGeomType geom_type = AccelStructGeomType::Triangles;
        uint8_t geometry_flags = static_cast<uint8_t>(AccelStructGeometryFlagBits::Opaque);

        VertexFormat vertex_format = VertexFormat::Invalid;
        buffer_handle vertex_buffer;
        uint32_t vertex_stride = 0; // zero means size of 'vertex_format'
        uint32_t num_vertices = 0;
        IndexFormat index_format = IndexFormat::Uint32;
        buffer_handle index_buffer;
        buffer_handle transform_buffer;
        buffer_handle instances_buffer;
        accel_struct_build_range build_range = {};
        uint8_t build_flags = static_cast<uint8_t>(AccelStructBuildFlagBits::PreferFastTrace);
        const char* debug_name = "";
    };

    struct dependencies
    {
        static constexpr auto s_max_submit_dependencies = 4;
        std::array<texture_handle, s_max_submit_dependencies> textures = {};
        std::array<buffer_handle, s_max_submit_dependencies> buffers = {};
    };

    struct submit_handle
    {
        uint32_t buffer_index = 0;
        uint32_t submit_id = 0;
        submit_handle() = default;
        explicit submit_handle(uint64_t handle)
            : buffer_index(static_cast<uint32_t>(handle)), submit_id(static_cast<uint32_t>(handle >> 32))
        {}
        [[nodiscard]] bool empty() const noexcept
        {
            return submit_id == 0;
        }
        uint64_t handle() const noexcept
        {
            return (static_cast<uint64_t>(submit_id) << 32) + buffer_index;
        }
    };
    static_assert(sizeof(submit_handle) == sizeof(uint64_t));
}
