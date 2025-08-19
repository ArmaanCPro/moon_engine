#pragma once

#include "renderer/device.h"

#include "vk.h"
#include "vk_render_types.h"

namespace moon::vulkan
{
    class vk_context;

    struct queue_families
    {
        static constexpr uint32_t s_invalid_index = std::numeric_limits<uint32_t>::max();

        vk::Queue graphics_queue;
        uint32_t graphics_queue_index = s_invalid_index;
        vk::Queue compute_queue;
        uint32_t compute_queue_index = s_invalid_index;
    };

    class vk_device final : public device
    {
    public:
        vk_device() = default;
        void init(vkb::Instance vkb_instance, vk::SurfaceKHR surface, vk_context* context);
        ~vk_device() override;

        [[nodiscard]] vk::PhysicalDevice get_physical_device() const { return m_physical_device; }
        [[nodiscard]] vk::Device get_device() const { return m_device.get(); }

        void record(vk::CommandBuffer cmd);

        [[nodiscard]] vk::Queue get_graphics_queue() const { return m_queue_families.graphics_queue; }
        [[nodiscard]] uint32_t get_graphics_queue_index() const { return m_queue_families.graphics_queue_index; }

        // sync primitives
        [[nodiscard]] vk::UniqueSemaphore create_semaphore() const;
        [[nodiscard]] vk::UniqueFence create_fence(bool start_open = true) const;
        void wait_for_fence(vk::Fence fence);
        void reset_fence(vk::Fence fence);

        // commands
        [[nodiscard]] vk::UniqueCommandPool create_command_pool(uint32_t queue_index, vk::CommandPoolCreateFlagBits flags) const;
        [[nodiscard]] vk::UniqueCommandBuffer allocate_command_buffer(vk::CommandPool pool,
            vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
        void reset_command_pool(vk::CommandPool pool);

        void immediate_submit(std::function<void(vk::CommandBuffer)>&& fn);

        vk::Result submit(vk::Queue queue, vk::CommandBuffer cmd, vk::Semaphore wait_semaphore,
                          vk::PipelineStageFlags2 wait_stage,
                          vk::Semaphore signal_semaphore, vk::Fence signal_fence);

        vk::Result submit_and_present(vk::CommandBuffer cmd, vk::Semaphore wait_semaphore, vk::Semaphore signal_semaphore,
                                vk::Fence signal_fence, vk::SwapchainKHR swapchain, uint32_t image_index);

        // image allocation
        allocated_image allocate_image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped = false) const;
        void destroy_image(allocated_image& image) const;

        // TEXTURES
        std::expected<vulkan_image, result> allocate_texture(texture_desc desc, const char* debug_name = nullptr);

        // BUFFERS
        vulkan_buffer allocate_buffer(vk::DeviceSize buffer_size, vk::BufferUsageFlags usage_flags,
                                      vk::MemoryPropertyFlags mem_flags, const char* debug_name = nullptr);

        VkDeviceAddress get_buffer_device_address(vk::Buffer buffer) const;

        // SAMPLERS
        vk::Sampler allocate_sampler(vk::SamplerCreateInfo ci, const char* debug_name = nullptr);


        [[nodiscard]] const vk::PhysicalDeviceProperties& get_physical_device_properties() const { return m_physical_device_properties; }
        [[nodiscard]] const std::vector<vk::Format>& get_supported_depth_formats() const { return m_supported_depth_formats; }
        [[nodiscard]] uint32_t get_framebuffer_msaa_bitmask() const;
        [[nodiscard]] bool has_acceleration_structure() const { return m_has_acceleration_structure; }
        [[nodiscard]] bool has_ray_query() const { return m_has_ray_query; }
        [[nodiscard]] bool has_ray_tracing_pipeline() const { return m_has_ray_tracing_pipeline; }
        [[nodiscard]] bool has_8bit_indices() const { return m_has_8bit_indices; }
        [[nodiscard]] bool has_calibrated_timestamps() const { return m_has_calibrated_timestamps; }
        [[nodiscard]] bool has_EXT_swapchain_maintenance1() const { return m_has_EXT_swapchain_maintenance1; }

        [[nodiscard]] VmaAllocator get_allocator() const { return m_allocator; }

        // HELPERS
        static constexpr std::vector<vk::Format> get_compatible_depth_stencil_formats(Format format);
        vk::Format get_closest_depth_stencil_format(Format format) const;

        // STAGING
        void buffer_subdata(vulkan_buffer& buffer, std::size_t offset, std::size_t size, const void* data);
        void image_data2D(vulkan_image& image, const vk::Rect2D& image_region, uint32_t base_mip_level,
            uint32_t num_mip_levels, uint32_t layer, uint32_t num_layers, vk::Format format, const void* data);
        void image_data3D(vulkan_image& image, const vk::Offset3D& offset,
                          const vk::Extent3D& extent, vk::Format format, const void* data);
        void get_image_data(vulkan_image& image, const vk::Offset3D& offset, const vk::Extent3D& extent,
            vk::ImageSubresourceRange range, vk::Format format, void* out_data);

    private:
        vk::PhysicalDevice m_physical_device;
        vk::UniqueDevice m_device;

        vk::PhysicalDeviceProperties m_physical_device_properties;
        std::vector<vk::Format> m_supported_depth_formats;

        queue_families m_queue_families;

        VmaAllocator m_allocator = VK_NULL_HANDLE;

        bool m_initialized = false;

        bool m_has_acceleration_structure = false;
        bool m_has_ray_query = false;
        bool m_has_ray_tracing_pipeline = false;
        bool m_has_8bit_indices = false;
        bool m_has_calibrated_timestamps = false;
        bool m_has_EXT_swapchain_maintenance1 = false;

        // staging
        static constexpr auto s_staging_buffer_alignment = 16u;
        struct memory_region_desc
        {
            uint32_t offset = 0;
            uint32_t size = 0;
            submit_handle hdl = {};
        };
        memory_region_desc get_next_free_offset(uint32_t size);
        void ensure_staging_buffer_size(uint32_t size);
        void wait_and_reset();
        holder<buffer_handle> m_staging_buffer;
        uint32_t m_staging_buffer_size = 0;
        uint32_t m_staging_buffer_counter = 0;
        uint32_t m_max_buffer_size = 0;
        const uint32_t m_min_buffer_size = 4u * 2048u * 2048u;
        std::deque<memory_region_desc> m_staging_memory_regions;
        vk_context* m_context;
    };
}
