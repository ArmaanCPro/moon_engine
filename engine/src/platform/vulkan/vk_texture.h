#pragma once

#include "renderer/texture.h"

#include "vk.h"
#include "vk_device.h"

#include <filesystem>

namespace moon
{
    class vk_texture2d final : public texture2d
    {
    public:
        vk_texture2d(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage_flags, vk_device& device, bool mipmapped = false);
        vk_texture2d(std::filesystem::path path, vk_device& device, bool mipmapped = false);
        ~vk_texture2d() override;

        uint32_t get_width() const override { return m_image.extent.width; }
        uint32_t get_height() const override { return m_image.extent.height; }
        uint32_t get_renderer_id() const override { return 0; }

        void set_data(void* data, uint32_t size) override;

        void bind(uint32_t slot) const override {}

        bool operator==(const texture& other) const override
        {
            return m_image.image == ((vk_texture2d&)other).m_image.image;
        }

    private:
        std::filesystem::path m_path;
        allocated_image m_image;

        // handle to allocator
        VmaAllocator m_allocator;
        VmaAllocation m_allocation;

        bool m_mipmapped = false;
    };
}
