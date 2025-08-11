#include "moonpch.h"
#include "vk_texture.h"

namespace moon
{
    vk_texture2d::vk_texture2d(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage_flags,
        vk_device& device, bool mipmapped)
        :
         m_allocator(device.get_allocator())
        , m_mipmapped(mipmapped)
    {
        vk::ImageCreateInfo imgCI{};
        imgCI.imageType = vk::ImageType::e2D;
        imgCI.format = format;
        imgCI.mipLevels = 1;
        if (mipmapped)
        {
            imgCI.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
        }
        imgCI.arrayLayers = 1;
        imgCI.samples = vk::SampleCountFlagBits::e1;
        imgCI.usage = usage_flags;

        m_image = device.allocate_image(vk::Extent3D{extent, 1}, format, usage_flags, mipmapped);
    }

    vk_texture2d::vk_texture2d(std::filesystem::path path, vk_device& device, bool mipmapped)
    {

    }

    vk_texture2d::~vk_texture2d()
    {
        vmaDestroyImage(m_allocator, m_image.get(), m_allocation);
    }

    void vk_texture2d::set_data(void* data, uint32_t size)
    {

    }
}
