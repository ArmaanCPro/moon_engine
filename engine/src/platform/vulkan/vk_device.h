#pragma once

#include "renderer/device.h"

#include "vk.h"

namespace moon
{
    struct queue_families
    {
        vk::Queue graphics_queue;
        uint32_t graphics_queue_index;
        vk::Queue transfer_queue;
        uint32_t transfer_queue_index;
    };

    class vk_device final : public device
    {
    public:
        vk_device() = default;
        void init(vkb::Instance vkb_instance, vk::SurfaceKHR surface);
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

        void immediate_submit(std::function<void(vk::CommandBuffer)> fn);

        vk::Result submit(vk::Queue queue, vk::CommandBuffer cmd, vk::Semaphore wait_semaphore,
                          vk::PipelineStageFlags2 wait_stage,
                          vk::Semaphore signal_semaphore, vk::Fence signal_fence);

        vk::Result submit_and_present(vk::CommandBuffer cmd, vk::Semaphore wait_semaphore, vk::Semaphore signal_semaphore,
                                vk::Fence signal_fence, vk::SwapchainKHR swapchain, uint32_t image_index);

        VkDeviceAddress get_buffer_device_address(vk::Buffer buffer) const;

        [[nodiscard]] VmaAllocator get_allocator() const { return m_allocator; }
    private:
        vk::PhysicalDevice m_physical_device;
        vk::UniqueDevice m_device;

        queue_families m_queue_families;

        VmaAllocator m_allocator = VK_NULL_HANDLE;

        bool m_initialized = false;
    };
}
