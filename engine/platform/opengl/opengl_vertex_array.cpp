#include "moonpch.h"

#include "opengl_vertex_array.h"
#include "moon/renderer/buffer.h"
#include <glad/glad.h>

namespace moon
{
    static GLenum shader_data_type_to_gl_type(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:     return GL_FLOAT;
        case ShaderDataType::Float2:    return GL_FLOAT;
        case ShaderDataType::Float3:    return GL_FLOAT;
        case ShaderDataType::Float4:    return GL_FLOAT;
        case ShaderDataType::Mat3:      return GL_FLOAT;
        case ShaderDataType::Mat4:      return GL_FLOAT;
        case ShaderDataType::Int:       return GL_INT;
        case ShaderDataType::Int2:      return GL_INT;
        case ShaderDataType::Int3:      return GL_INT;
        case ShaderDataType::Int4:      return GL_INT;
        case ShaderDataType::Bool:      return GL_BOOL;
        default: MOON_CORE_ASSERT(false, "Unknown ShaderDataType!"); return 0;
        }
    }

    opengl_vertex_array::opengl_vertex_array()
    {
        glCreateVertexArrays(1, &renderer_id_);
    }

    opengl_vertex_array::~opengl_vertex_array()
    {
        glDeleteVertexArrays(1, &renderer_id_);
    }

    void opengl_vertex_array::bind() const
    {
        glBindVertexArray(renderer_id_);
    }

    void opengl_vertex_array::unbind() const
    {
        glBindVertexArray(0);
    }

    void opengl_vertex_array::add_vertex_buffer(ref<vertex_buffer> vbuf)
    {
        MOON_CORE_ASSERT(!vbuf->get_layout().get_elements().empty(), "Vertex Buffer has no layout!");
        glBindVertexArray(renderer_id_);
        vbuf->bind();

        uint32_t buffer_index = 0;
        const auto& layout = vbuf->get_layout();
        for (const auto& element : layout)
        {
            glVertexAttribPointer(buffer_index,
                (GLint)element.get_component_count(),
                shader_data_type_to_gl_type(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                (GLint)layout.get_stride(),
                (const void*)element.offset
            );

            glEnableVertexAttribArray(buffer_index);
            buffer_index++;
        }

        vertex_buffers_.push_back(vbuf);
    }

    void opengl_vertex_array::set_index_buffer(ref<index_buffer> ibuf)
    {
        glBindVertexArray(renderer_id_);
        ibuf->bind();

        index_buffer_ = ibuf;
    }
}
