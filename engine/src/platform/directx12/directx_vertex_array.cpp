#include "moonpch.h"
#include "directx_vertex_array.h"

#include "directx.h"

namespace moon
{
    // to be used probably in the graphics pipeline. temporarily here
    static DXGI_FORMAT shader_data_type_to_dxgi_format(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float: return DXGI_FORMAT_R32_FLOAT;
            case ShaderDataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
            case ShaderDataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case ShaderDataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case ShaderDataType::Mat3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case ShaderDataType::Mat4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case ShaderDataType::Int: return DXGI_FORMAT_R32_SINT;
            case ShaderDataType::Int2: return DXGI_FORMAT_R32G32_SINT;
            case ShaderDataType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
            case ShaderDataType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;
            case ShaderDataType::Bool: return DXGI_FORMAT_R32_UINT;
            default: MOON_CORE_ASSERT(false, "Unknown ShaderDataType!"); return DXGI_FORMAT_UNKNOWN;
        }
    }

    void directx_vertex_array::bind() const
    {
        MOON_PROFILE_FUNCTION();

        for (const auto& vbuf : m_vertex_buffers)
        {
            vbuf->bind();
        }
        if (m_index_buffer)
        {
            m_index_buffer->bind();
        }
    }

    void directx_vertex_array::add_vertex_buffer(ref<vertex_buffer> vbuf)
    {
        m_vertex_buffers.push_back(vbuf);
    }

    void directx_vertex_array::set_index_buffer(ref<index_buffer> ibuf)
    {
        m_index_buffer = ibuf;
    }
}
