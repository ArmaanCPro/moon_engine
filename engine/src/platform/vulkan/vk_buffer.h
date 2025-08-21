#pragma once

#include "vulkan/vk.h"
#include "renderer/buffer.h"

namespace moon::vulkan
{
    class vk_context;

    class vk_vertex_buffer final : public vertex_buffer
    {
    public:
        explicit vk_vertex_buffer(uint32_t size, vk_context& context);
        explicit vk_vertex_buffer(const float* vertices, uint32_t size, vk_context& context);
        ~vk_vertex_buffer() override = default;

        void set_data(const void* data, uint32_t size) override;

        void bind() const override {};
        void unbind() const override {};

        [[nodiscard]] const vertex_input& get_layout() const override { return m_layout; }
        void set_layout(const vertex_input& layout) override { m_layout = layout; }

    private:
        holder<buffer_handle> m_buffer;
        vk_context& m_context;
        // TODO vertex_input shouldn't be owned by the vertex_buffer, that belongs to the pipeline
        vertex_input m_layout;
    };

    class vk_index_buffer final : public index_buffer
    {
    public:
        vk_index_buffer(uint32_t count, vk_context& context);
        vk_index_buffer(uint32_t* indices, uint32_t size, vk_context& context);
        ~vk_index_buffer() override = default;

        uint32_t get_count() const override { return m_count; }

        void bind() const override {};
        void unbind() const override {};

    private:
        holder<buffer_handle> m_buffer;
        vk_context& m_context;
        uint32_t m_count;
    };
}
