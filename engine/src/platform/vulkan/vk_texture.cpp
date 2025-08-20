#include "moonpch.h"
#include "vk_texture.h"

#include "vk_context.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace moon::vulkan
{
    vk_texture2d::vk_texture2d(dimensions dims, vk_context& context, TextureUsageBits usages, bool mipmapped)
        :
        m_texture(context.create_texture({
                                             .type = TextureType::e2D,
                                             .format = Format::RGBA_UN8,
                                             .dimensions = dims,
                                             .usage = static_cast<uint8_t>(usages),
                                             .data = nullptr,
                                             .debug_name = "vk_texture2d no path"
                                         }, nullptr).value())
        , m_width(dims.width)
        , m_height(dims.height)
        , m_context(context)
        , m_device(context.get_device())
        , m_mipmapped(mipmapped)
    {}

    vk_texture2d::vk_texture2d(const std::filesystem::path& path, vk_context& context, bool mipmapped)
        :
        m_context(context)
        , m_device(context.get_device())
        , m_mipmapped(mipmapped)
    {
        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        uint8_t* data = stbi_load(absolute(path).string().c_str(), &width, &height, &channels, 4);
        MOON_CORE_ASSERT(data);
        m_width = static_cast<uint32_t>(width);
        m_height = static_cast<uint32_t>(height);

        m_texture = m_context.create_texture({
            .type = TextureType::e2D,
            .format = Format::RGBA_UN8,
            .dimensions = { m_width, m_height, 1 },
            .usage = static_cast<uint8_t>(TextureUsageBits::Sampled),
            .data = data,
            .debug_name = "vk_texture2d from path"
        }, nullptr).value();

        stbi_image_free(static_cast<void*>(data));
    }

    void vk_texture2d::set_data(void* data, uint32_t size)
    {
        MOON_CORE_ASSERT_MSG(size == m_width * m_height * 4, "Size must match the original size of the texture!");

        m_context.upload_texture(m_texture, {
            .dimensions = { m_width, m_height, 1 }
        }, data);
    }
}
