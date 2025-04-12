#pragma once

namespace moon
{
    class MOON_API vertex_buffer
    {
    public:
        virtual ~vertex_buffer() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        static vertex_buffer* create(const float* vertices, uint32_t size);
    };

    class MOON_API index_buffer
    {
    public:
        virtual ~index_buffer() = default;

        virtual uint32_t get_count() const = 0;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        static index_buffer* create(const uint32_t* indices, uint32_t count);
    };
}
