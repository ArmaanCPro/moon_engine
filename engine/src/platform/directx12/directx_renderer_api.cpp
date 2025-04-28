#include "moonpch.h"
#include "directx_renderer_api.h"

#include "directx.h"
#include "core/application.h"

namespace moon
{
    // maybe temporary
    struct data
    {
        directx_context* context;
    };
    static data s_data;

    void directx_renderer_api::init()
    {
        s_data.context = (directx_context*)application::get().get_window().get_context();
    }

    void directx_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {

    }

    void directx_renderer_api::set_clear_color(const glm::vec4& color)
    {
        s_data.context->set_clear_color(color);
    }

    void directx_renderer_api::clear()
    {
        s_data.context->clear();
    }

    void directx_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {

    }
}
