#pragma once

#include "renderer/texture.h"

#include "vk.h"

#include <filesystem>

#include "renderer/render_types.h"

namespace moon::vulkan
{
    class vk_context;
    class vk_device;

    class vk_texture2d final : public texture2d
    {
    public:
        vk_texture2d(dimensions dims, vk_context& context, TextureUsageBits usages = TextureUsageBits::Sampled, bool mipmapped = false);
        vk_texture2d(const std::filesystem::path& path, vk_context& context, bool mipmapped = false);
        ~vk_texture2d() override = default;

        [[nodiscard]] uint32_t get_width() const override { return m_width; }
        [[nodiscard]] uint32_t get_height() const override { return m_height; }
        [[nodiscard]] uint32_t get_renderer_id() const override { return 0; }

        void set_data(void* data, [[maybe_unused]] uint32_t size) override;

        texture_handle get_handle() override { return m_texture; }

        void bind(uint32_t slot) const override {}

        bool operator==(const texture& other) const override
        {
            return m_texture == ((vk_texture2d&)other).m_texture;
        }

    private:
        holder<texture_handle> m_texture;
        uint32_t m_width = 0;
        uint32_t m_height = 0;

        vk_context& m_context;
        const vk_device& m_device;

        bool m_mipmapped = false;
    };
}
