#pragma once

#include "moon/renderer/vertex_array.h"

namespace moon
{
    class d3d12_vertex_array : public vertex_array
    {
    public:
        d3d12_vertex_array() = default;
        ~d3d12_vertex_array() override = default;

        void bind() const override;
        void unbind() const override {}

        void add_vertex_buffer(ref<vertex_buffer> vbuf) override;
        void set_index_buffer(ref<index_buffer> ibuf) override;

        const std::vector<ref<vertex_buffer>>& get_vertex_buffers() const override { return m_vertex_buffers; }
        const ref<index_buffer>& get_index_buffer() const override { return m_index_buffer; }

    private:
        std::vector<ref<vertex_buffer>> m_vertex_buffers;
        ref<index_buffer> m_index_buffer;
    };
}
