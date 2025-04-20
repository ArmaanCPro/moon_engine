#pragma once

#include "core/core.h"
#include "renderer/texture.h"

#include <glm/vec2.hpp>

namespace moon
{
    class MOON_API subtexture2d
    {
    public:
        subtexture2d(const ref<texture2d>& texture, const glm::vec2& min_coords, const glm::vec2& max_coords);

        const ref<texture2d>& get_texture() const { return m_texture; }
        const glm::vec2* get_texcoords() const { return m_texcoords; }

        static ref<subtexture2d> create_from_coords(const ref<texture2d>& texture, const glm::vec2& coords, const glm::vec2& cell_size, const glm::vec2& sprite_size = { 1, 1 });
    private:
        ref<texture2d> m_texture;

        glm::vec2 m_texcoords[4];
    };
}
