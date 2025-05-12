#pragma once

#include "moon/renderer/shader.h"
#include "d3d12_include.h"

namespace moon
{
    class d3d12_shader : public shader
    {
    public:
        explicit d3d12_shader(ShaderType type, std::string_view filepath);
        d3d12_shader(std::string_view vertex_path, std::string_view fragment_path);
        d3d12_shader(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);
        ~d3d12_shader() override;

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

        void init_root_signature();
        void init_constant_buffers();
        void init_shader_resource_views();

    private:
        std::string m_name;
        std::string m_data;
        // embedded rootsig
        ComPtr<ID3D12RootSignature> m_root_signature;

        // DX specific resources
        ComPtr<ID3D12DescriptorHeap> m_srv_descriptor_heap;
        ComPtr<ID3D12Resource2> m_null_texture; // null srv resource for initialization purposes

        ComPtr<ID3D12DescriptorHeap> m_cbv_descriptor_heap;
        std::vector<ComPtr<ID3D12Resource2>> m_constant_buffers;

        // Persistent mapping for the CBV's mat4
        UINT8* m_cbv_mapped_data = nullptr;
    };
}
