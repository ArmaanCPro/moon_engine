#include "moonpch.h"
#include "vk_renderer_api.h"

namespace moon
{
    void vk_renderer_api::init()
    {

    }

    void vk_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {

    }

    void vk_renderer_api::set_clear_color(const glm::vec4& color)
    {
        m_clear_color = color;
    }

    void vk_renderer_api::clear()
    {

    }

    void vk_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {
        
    }
}
