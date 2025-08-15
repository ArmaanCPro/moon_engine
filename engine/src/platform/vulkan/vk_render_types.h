#pragma once

#include "renderer/render_types.h"
#include "vulkan/vk.h"

namespace moon::vulkan
{
    class vk_context;

    struct vulkan_buffer final
    {
        [[nodiscard]] inline uint8_t* get_mapped_ptr() const { return static_cast<uint8_t*>(m_mapped_ptr); }
        [[nodiscard]] inline bool is_mapped() const { return m_mapped_ptr != nullptr; }

        void buffer_subdata(vk_context& context, std::size_t offset, std::size_t size, const void* data);
        void get_buffer_subdata(vk_context& context, std::size_t offset, std::size_t size, void* data);
        // cpu -> gpu
        void flush_mapped_memory(vk_context& context, vk::DeviceSize offset, vk::DeviceSize size) const;
        // gpu -> cpu
        void invalidate_mapped_memory(vk_context& context, vk::DeviceSize offset, vk::DeviceSize size) const;

    public:
        vk::Buffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        vk::DeviceSize m_size = 0;
        void* m_mapped_ptr = nullptr;
        bool m_is_coherent_memory = false;
    };

    struct vulkan_image final
    {
        // clang-format off
        [[nodiscard]] inline bool is_sampled_image() const { return (m_usage_flags & vk::ImageUsageFlagBits::eSampled) != vk::ImageUsageFlags{}; }
        [[nodiscard]] inline bool is_storage_image() const { return (m_usage_flags & vk::ImageUsageFlagBits::eStorage) != vk::ImageUsageFlags{}; }
        [[nodiscard]] inline bool is_color_attachment() const { return (m_usage_flags & vk::ImageUsageFlagBits::eColorAttachment) != vk::ImageUsageFlags{}; }
        [[nodiscard]] inline bool is_depth_attachment() const { return (m_usage_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment) != vk::ImageUsageFlags{}; }
        [[nodiscard]] inline bool is_attachment() const { return is_color_attachment() || is_depth_attachment(); }
        // clang-format on

        // setting num levels to a non-zero value will override mip_levels from the original vulkan_image, and can be
        // used to create image views with different mip levels
        [[nodiscard]] vk::ImageView create_image_view( vk::Device device, vk::ImageViewType type, vk::Format format,
            vk::ImageAspectFlags aspect_mask, uint32_t base_level, uint32_t num_levels = vk::RemainingMipLevels,
            uint32_t base_layer = 0, uint32_t num_layers = 1, const vk::ComponentMapping mapping = {},
            const vk::SamplerYcbcrConversionInfo* ycbcr = nullptr, const char* debug_name = nullptr) const;

        void generate_mipmap(vk::CommandBuffer cmd) const;
        void transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout, const vk::ImageSubresourceRange& subresource_range) const;

        [[nodiscard]] vk::ImageAspectFlags get_image_aspect_flags() const;

        // framebuffers can render only into one level/layer
        [[nodiscard]] vk::ImageView get_or_create_image_view_for_framebuffer(vk_context& context, uint32_t level, uint32_t layer);

        [[nodiscard]] static constexpr bool is_depth_format(vk::Format format);
        [[nodiscard]] static constexpr bool is_stencil_format(vk::Format format);

    public:
        vk::Image m_image = VK_NULL_HANDLE;
        vk::ImageUsageFlags m_usage_flags = {};
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        vk::FormatProperties m_format_properties = {};
        vk::Format m_format = vk::Format::eUndefined;
        vk::Extent3D m_extent = {};
        vk::ImageType m_type = vk::ImageType::e2D;
        vk::SampleCountFlagBits m_samples = vk::SampleCountFlagBits::e1;
        void* m_mapped_ptr = nullptr;
        bool m_is_swapchain_image = false;
        bool m_is_owning_vk_image = true;
        bool m_is_resolve_attachment = false; // autoset by cmdBeginRendering() for extra synchronization
        uint32_t m_num_levels = 1u;
        uint32_t m_num_layers = 1u;
        bool m_is_depth_format = false;
        bool m_is_stencil_format = false;
        char m_debug_name[256] = {};

        // current image layout
        mutable vk::ImageLayout m_image_layout = vk::ImageLayout::eUndefined;
        // precached image views; owned by this vulkan_image
        vk::ImageView m_image_view = VK_NULL_HANDLE; // default view w/ all mip levels
        vk::ImageView m_image_view_storage = VK_NULL_HANDLE; // view w/ identity swizzle (all mip levels)
        std::array<std::array<vk::ImageView, 6>, s_max_mip_levels> m_image_view_for_framebuffer = {}; // // max 6 faces for cubemap rendering
    };
}
