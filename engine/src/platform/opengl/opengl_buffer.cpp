#include "moonpch.h"

#include "opengl_buffer.h"

#include <glad/glad.h>

namespace moon
{
    // ////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////

    opengl_vertex_buffer::opengl_vertex_buffer(uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        glCreateBuffers(1, &renderer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    opengl_vertex_buffer::opengl_vertex_buffer(const float* vertices, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        glCreateBuffers(1, &renderer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    opengl_vertex_buffer::~opengl_vertex_buffer()
    {
        MOON_PROFILE_FUNCTION();

        glDeleteBuffers(1, &renderer_id_);
    }

    void opengl_vertex_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    }

    void opengl_vertex_buffer::unbind() const
    {
        MOON_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void opengl_vertex_buffer::set_data(const void* data, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }

    // ////////////////////////////////////////////////
    // INDEX BUFFER ///////////////////////////////////

    opengl_index_buffer::opengl_index_buffer(const uint32_t* indices, uint32_t count)
        :
        count_(count)
    {
        MOON_PROFILE_FUNCTION();

        glCreateBuffers(1, &renderer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    opengl_index_buffer::~opengl_index_buffer()
    {
        MOON_PROFILE_FUNCTION();

        glDeleteBuffers(1, &renderer_id_);
    }

    void opengl_index_buffer::bind() const
    {
        MOON_PROFILE_FUNCTION();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    }

    void opengl_index_buffer::unbind() const
    {
        MOON_PROFILE_FUNCTION();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
