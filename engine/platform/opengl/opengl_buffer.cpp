#include "moonpch.h"

#include "opengl_buffer.h"

#include <glad/glad.h>

namespace moon
{
    // ////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////

    opengl_vertex_buffer::opengl_vertex_buffer(const float* vertices, uint32_t size)
    {
        glCreateBuffers(1, &renderer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

    opengl_vertex_buffer::~opengl_vertex_buffer()
    {
        glDeleteBuffers(1, &renderer_id_);
    }

    void opengl_vertex_buffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    }

    void opengl_vertex_buffer::unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // ////////////////////////////////////////////////
    // INDEX BUFFER ///////////////////////////////////

    opengl_index_buffer::opengl_index_buffer(const uint32_t* indices, uint32_t count)
        :
        count_(count)
    {
        glCreateBuffers(1, &renderer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    opengl_index_buffer::~opengl_index_buffer()
    {}

    void opengl_index_buffer::bind() const
    {}

    void opengl_index_buffer::unbind() const
    {}
}
