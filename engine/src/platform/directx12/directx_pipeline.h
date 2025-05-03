#pragma once

#include "moon/renderer/pipeline.h"
#include "directx.h"

namespace moon
{
    class directx_pipeline : public pipeline
    {
    public:
        directx_pipeline(const pipeline_spec& spec);
        ~directx_pipeline() override;

        void bind() override;
        void unbind() override {}

    private:
        ComPtr<ID3D12PipelineState> m_pipeline_state;
    };
}
