#pragma once

#include "moon/renderer/buffer.h"

namespace moon
{
    class opengl_vertex_buffer : public vertex_buffer
    {
    public:
        opengl_vertex_buffer(uint32_t size);
        opengl_vertex_buffer(const float* vertices, uint32_t size);
        ~opengl_vertex_buffer() override;

        void bind() const override;
        void unbind() const override;

        void set_data(const void* data, uint32_t size) override;

        const vertex_input& get_layout() const override { return layout_; }
        void set_layout(const vertex_input& layout) override { layout_ = layout; }

    private:
        uint32_t renderer_id_{0};
        vertex_input layout_;
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
