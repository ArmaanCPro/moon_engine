#pragma once

#include <vector>
#include "renderer/render_types.h"

namespace moon
{
    class MOON_API vertex_buffer
    {
    public:
        virtual ~vertex_buffer() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void set_data(const void* data, uint32_t size) = 0;

        virtual const vertex_input& get_layout() const = 0;
        virtual void set_layout(const vertex_input& layout) = 0;

        static scope<vertex_buffer> create(uint32_t size);
        static scope<vertex_buffer> create(const float* vertices, uint32_t size);
    };

    // Currently, only 32-bit index buffers are supported (TODO: use IndexFormat)
    class MOON_API index_buffer
    {
    public:
        virtual ~index_buffer() = default;

        virtual uint32_t get_count() const = 0;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        static ref<index_buffer> create(const uint32_t* indices, uint32_t count);
    };
}
