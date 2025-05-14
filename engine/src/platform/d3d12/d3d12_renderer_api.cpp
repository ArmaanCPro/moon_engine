#include "moonpch.h"
#include "d3d12_renderer_api.h"

#include "d3d12_include.h"
#include "core/application.h"

namespace moon
{
    // maybe temporary
    struct data
    {
        d3d12_context* context;
    };
    static data s_data;

    void d3d12_renderer_api::init()
    {
        s_data.context = (d3d12_context*)application::get().get_context();
    }

    void d3d12_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        ((d3d12_context*)application::get().get_context())->on_resize(width, height);
    }

    void d3d12_renderer_api::clear(const glm::vec4& color)
    {
        s_data.context->clear(color);
    }

    void d3d12_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {
        s_data.context->get_command_list(s_data.context->get_current_buffer_index())->draw_indexed(vertex_array, index_count);
    }
}
