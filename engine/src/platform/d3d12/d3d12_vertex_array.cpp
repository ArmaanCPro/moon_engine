#include "moonpch.h"
#include "d3d12_vertex_array.h"

#include "d3d12_include.h"

namespace moon
{
    void d3d12_vertex_array::bind() const
    {
        MOON_PROFILE_FUNCTION();

        for (const auto& vbuf : m_vertex_buffers)
        {
            // consider reducing this to 1 api call by binding all the views in one cmdlist call
            vbuf->bind();
        }
        if (m_index_buffer)
        {
            m_index_buffer->bind();
        }
    }

    void d3d12_vertex_array::add_vertex_buffer(ref<vertex_buffer> vbuf)
    {
        m_vertex_buffers.push_back(vbuf);
    }

    void d3d12_vertex_array::set_index_buffer(ref<index_buffer> ibuf)
    {
        m_index_buffer = ibuf;
    }
}
