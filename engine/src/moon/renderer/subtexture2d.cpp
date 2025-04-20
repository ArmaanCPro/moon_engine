#include "subtexture2d.h"

namespace moon
{

    subtexture2d::subtexture2d(const ref<texture2d>& texture, const glm::vec2& min_coords, const glm::vec2& max_coords)
        :
        m_texture(texture)
    {
        m_texcoords[0] = { min_coords.x, min_coords.y };
        m_texcoords[1] = { max_coords.x, min_coords.y };
        m_texcoords[2] = { max_coords.x, max_coords.y };
        m_texcoords[3] = { min_coords.x, max_coords.y };
    }

    ref<subtexture2d> subtexture2d::create_from_coords(const ref<texture2d>& texture, const glm::vec2& coords,
        const glm::vec2& cell_size, const glm::vec2& sprite_size)
    {
        float texture_width = (float)texture->get_width();
        float texture_height = (float)texture->get_height();

        glm::vec2 min = { (coords.x + cell_size.x) / texture_width, (coords.y + cell_size.y) / texture_height };
        glm::vec2 max = { ((coords.x + sprite_size.x) * cell_size.x) / texture_width, ((coords.y + sprite_size.y) + cell_size.y) / texture_height };

        return create_ref<subtexture2d>(texture, min, max);
    }
}
