#include "moonpch.h"
#include "directx_pipeline.h"

#include "directx_context.h"
#include "core/application.h"

namespace moon
{
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

    static std::vector<D3D12_INPUT_ELEMENT_DESC> buffer_layout_to_d3d_layout(const buffer_layout& layout, UINT input_slot = 0)
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> d3d_layout = {};
        d3d_layout.reserve(layout.get_elements().size());

        auto& elements = layout.get_elements();
        for (const auto& element : elements)
        {
            DXGI_FORMAT format = shader_data_type_to_dxgi_format(element.type);
            MOON_CORE_ASSERT(format != DXGI_FORMAT_UNKNOWN, "Invalid ShaderDataType!");

            if (element.type == ShaderDataType::Mat3 || element.type == ShaderDataType::Mat4)
            {
                // For matrix types, split into multiple rows (1 row = 1 input element)
                uint32_t rowCount = (element.type == ShaderDataType::Mat3) ? 3 : 4; // Mat3 has 3 rows, Mat4 has 4
                for (uint32_t row = 0; row < rowCount; ++row)
                {
                    D3D12_INPUT_ELEMENT_DESC desc = {};
                    desc.SemanticName = element.name.c_str();
                    desc.SemanticIndex = row; // row as semantic index for matrices
                    desc.Format = (element.type == ShaderDataType::Mat3) ? DXGI_FORMAT_R32G32B32_FLOAT : DXGI_FORMAT_R32G32B32A32_FLOAT;
                    desc.InputSlot = input_slot;
                    desc.AlignedByteOffset = (UINT)(element.offset + row * sizeof(float) * 3);
                    desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    desc.InstanceDataStepRate = 0;
                    d3d_layout.push_back(std::move(desc));
                }
            }
            else
            {
                // non-matrices
                D3D12_INPUT_ELEMENT_DESC desc = {};
                desc.SemanticName = element.name.c_str();
                desc.SemanticIndex = 0;
                desc.Format = format;
                desc.InputSlot = input_slot;
                desc.AlignedByteOffset = (UINT)element.offset;
                desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                desc.InstanceDataStepRate = 0;
                d3d_layout.push_back(std::move(desc));
            }
        }
        return d3d_layout;
    }

    directx_pipeline::directx_pipeline(const pipeline_spec& spec)
    {
        MOON_PROFILE_FUNCTION();

        if (spec.rootsig_shader)
        {
            MOON_CORE_ASSERT(false, "Root signature shader is null!");
        }

        directx_context* context = (directx_context*)application::get().get_context();

        ComPtr<ID3D12RootSignature> root_signature;
        if (spec.rootsig_shader)
        {
            std::string_view rootsig_data = spec.rootsig_shader->get_data();
            context->get_device()->CreateRootSignature(0, rootsig_data.data(), rootsig_data.size(), IID_PPV_ARGS(&root_signature));
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psod = {};
        psod.pRootSignature = root_signature.Get();

        // TODO: Get all unique buffer layouts in the vertex array and accordingly set the InputSlot
        auto input_layout = buffer_layout_to_d3d_layout(spec.vertex_array->get_vertex_buffers()[0]->get_layout());

        psod.InputLayout.NumElements = (UINT)input_layout.size();
        psod.InputLayout.pInputElementDescs = input_layout.data();

        psod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

        if (spec.vertex_shader && spec.fragment_shader)
        {
            psod.VS.pShaderBytecode = spec.vertex_shader->get_data().data();
            psod.VS.BytecodeLength = spec.vertex_shader->get_data().size();
            psod.PS.pShaderBytecode = spec.fragment_shader->get_data().data();
            psod.PS.BytecodeLength = spec.fragment_shader->get_data().size();
        }
        else
        {
            MOON_CORE_ERROR("Vertex or Fragment shader is null!");
        }

        // TODO: rest of these shaders
        psod.DS.pShaderBytecode = nullptr;
        psod.DS.BytecodeLength = 0;
        psod.GS.pShaderBytecode = nullptr;
        psod.GS.BytecodeLength = 0;
        psod.HS.pShaderBytecode = nullptr;
        psod.HS.BytecodeLength = 0;

        psod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psod.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psod.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        psod.RasterizerState.FrontCounterClockwise = FALSE;
        psod.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        psod.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        psod.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        psod.RasterizerState.DepthClipEnable = FALSE;
        psod.RasterizerState.MultisampleEnable = FALSE;
        psod.RasterizerState.AntialiasedLineEnable = FALSE;
        psod.RasterizerState.ForcedSampleCount = 0;

        psod.StreamOutput.NumEntries = 0;
        psod.StreamOutput.NumStrides = 0;
        psod.StreamOutput.pBufferStrides = nullptr;
        psod.StreamOutput.pSODeclaration = nullptr;
        psod.StreamOutput.RasterizedStream = 0;

        psod.NumRenderTargets = 1;
        psod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psod.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psod.BlendState.AlphaToCoverageEnable = FALSE;
        psod.BlendState.IndependentBlendEnable = FALSE;
        psod.BlendState.RenderTarget[0].BlendEnable = FALSE;
        psod.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
        psod.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        psod.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        psod.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        psod.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        psod.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        psod.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        psod.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        psod.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psod.DepthStencilState.DepthEnable = FALSE;
        psod.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        psod.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        psod.DepthStencilState.StencilEnable = FALSE;
        psod.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        psod.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        psod.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        psod.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        psod.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        psod.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        psod.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        psod.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        psod.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        psod.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

        psod.SampleMask = UINT_MAX;
        psod.SampleDesc.Count = 1;
        psod.SampleDesc.Quality = 0;

        psod.NodeMask = 0;
        psod.CachedPSO.CachedBlobSizeInBytes = 0;
        psod.CachedPSO.pCachedBlob = nullptr;
        psod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        context->get_device()->CreateGraphicsPipelineState(&psod, IID_PPV_ARGS(&m_pipeline_state));
    }

    directx_pipeline::~directx_pipeline()
    {
        m_pipeline_state.Reset();
    }

    void directx_pipeline::bind()
    {
        directx_context* context = (directx_context*)application::get().get_context();
        context->get_native_command_list()->SetPipelineState(m_pipeline_state.Get());
    }
}
