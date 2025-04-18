#pragma once

#include "buffer.h"
#include <memory>

namespace moon
{
    class MOON_API vertex_array
    {
    public:
        virtual ~vertex_array() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void add_vertex_buffer(ref<vertex_buffer> vbuf) = 0;
        virtual void set_index_buffer(ref<index_buffer> ibuf) = 0;

        virtual const std::vector<ref<vertex_buffer>>& get_vertex_buffers() const = 0;
        virtual const ref<index_buffer>& get_index_buffer() const = 0;

        static ref<vertex_array> create();
    };
}
