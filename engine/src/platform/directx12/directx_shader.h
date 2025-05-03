#pragma once

#include "moon/renderer/shader.h"
#include "directx.h"

namespace moon
{
    class directx_shader : public shader
    {
    public:
        explicit directx_shader(ShaderType type, std::string_view filepath);
        directx_shader(std::string_view vertex_path, std::string_view fragment_path);
        directx_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);
        ~directx_shader() override;

        std::string_view get_data() override { return m_data; }
        std::string_view get_name() override { return m_name; }
        const ComPtr<ID3D12RootSignature>& get_root_signature() const { return m_root_signature; }

        ComPtr<ID3DBlob> create_blob() const;

        void bind() const override;
        void unbind() const override;

        void set_int(std::string_view name, int value) override;
        void set_int_array(std::string_view name, int* values, uint32_t count) override;
        void set_float(std::string_view name, float value) override;
        void set_float2(std::string_view name, const glm::vec2& value) override;
        void set_float3(::std::string_view name, const glm::vec3& value) override;
        void set_float4(::std::string_view name, const glm::vec4& value) override;
        void set_mat4(::std::string_view name, const glm::mat4& value) override;

    private:
        std::string read_file(std::string_view filepath);

    private:
        std::string m_name;
        std::string m_data;
        // embedded rootsig
        ComPtr<ID3D12RootSignature> m_root_signature;

        // DX specific resources
        ComPtr<ID3D12DescriptorHeap> m_srv_descriptor_heap;

        ComPtr<ID3D12DescriptorHeap> m_cbv_descriptor_heap;
        std::vector<ComPtr<ID3D12Resource2>> m_constant_buffers;
    };
}
