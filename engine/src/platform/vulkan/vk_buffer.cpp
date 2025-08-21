#include "moonpch.h"
#include "vk_buffer.h"

#include "vk_context.h"

namespace moon::vulkan
{
    vk_vertex_buffer::vk_vertex_buffer(uint32_t size, vk_context& context)
        :
        m_buffer(context.create_buffer({
            .usage = static_cast<uint8_t>(BufferUsageBits::Vertex),
            .storage_type = StorageType::Device,
            .size = size,
            .debug_name = "vk_vertex_buffer"
        }).value())
        , m_context(context)
    {
        // TODO use storage buffer for vertex/index buffers
    }

    vk_vertex_buffer::vk_vertex_buffer(const float* vertices, uint32_t size, vk_context& context)
        :
        m_buffer(context.create_buffer({
            .usage = static_cast<uint8_t>(BufferUsageBits::Index),
            .storage_type = StorageType::Device,
            .size = size,
            .data = vertices,
            .debug_name = "vk_vertex_buffer"
        }).value())
        , m_context(context)
    {}

    void vk_vertex_buffer::set_data(const void* data, uint32_t size)
    {
        m_context.upload_buffer(m_buffer, data, size, 0);
    }

    vk_index_buffer::vk_index_buffer(uint32_t count, vk_context& context)
        :
        m_buffer(context.create_buffer({
            .usage = static_cast<uint8_t>(BufferUsageBits::Index),
            .storage_type = StorageType::Device,
            .size = count, // TODO this might have to be count * sizeof(uint32)t
            .debug_name = "vk_index_buffer"
        }).value())
        , m_context(context)
        , m_count(count)
    {}

    vk_index_buffer::vk_index_buffer(uint32_t* indices, uint32_t size, vk_context& context)
        :
        m_buffer(context.create_buffer({
            .usage = static_cast<uint8_t>(BufferUsageBits::Index),
            .storage_type = StorageType::Device,
            .size = size,
            .data = indices,
            .debug_name = "vk_index_buffer"
        }).value())
        , m_context(context)
        , m_count(size)
    {}
}
