#pragma once

#include "moon/renderer/pipeline.h"
#include "d3d12_include.h"

namespace moon
{
    class d3d12_pipeline : public pipeline
    {
    public:
        d3d12_pipeline(const pipeline_spec& spec);
        ~d3d12_pipeline() override;

        void bind() override;
        void unbind() override {}

    private:
        ComPtr<ID3D12PipelineState> m_pipeline_state;
    };
}
