#pragma once

#include "core/core.h"
#include "core/renderer/buffer.h"

namespace moon
{
    class opengl_vertex_buffer : public vertex_buffer
    {
    public:
        opengl_vertex_buffer(const float* vertices, uint32_t size);
        ~opengl_vertex_buffer() override;

        void bind() const override;
        void unbind() const override;
    private:
        uint32_t renderer_id_{0};
    };

    class opengl_index_buffer : public index_buffer
    {
    public:
        opengl_index_buffer(const uint32_t* indices, uint32_t count);
        ~opengl_index_buffer() override;

        inline uint32_t get_count() const override { return count_; }

        void bind() const override;
        void unbind() const override;
    private:
        uint32_t renderer_id_{0};
        uint32_t count_;
    };
}
