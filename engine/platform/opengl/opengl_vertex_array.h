#pragma once

#include "core/renderer/vertex_array.h"

namespace moon
{
    class opengl_vertex_array : public vertex_array
    {
    public:
        opengl_vertex_array();
        ~opengl_vertex_array() override;

        void bind() const override;
        void unbind() const override;

        void add_vertex_buffer(std::shared_ptr<vertex_buffer> vbuf) override;
        void set_index_buffer(std::shared_ptr<index_buffer> ibuf) override;

        const std::vector<std::shared_ptr<vertex_buffer>>& get_vertex_buffers() const override { return vertex_buffers_; }
        const std::shared_ptr<index_buffer>& get_index_buffer() const override { return index_buffer_; }
    private:
        std::vector<std::shared_ptr<vertex_buffer>> vertex_buffers_;
        std::shared_ptr<index_buffer> index_buffer_;
        uint32_t renderer_id_{0};
    };
}
