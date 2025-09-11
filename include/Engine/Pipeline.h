/// @file Pipeline.h
/// @brief ルートシグニチャとPSO

#include <d3d12.h>

#include <vector>

#include "Engine/ComPtr.h"

class Pipeline {
    engine::ComPtr<ID3D12PipelineState> m_pPipelineState;

public:
    Pipeline() = default;
    ~Pipeline() = default;

    bool Create(
        ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
    ID3D12PipelineState* Get() const { return m_pPipelineState.Get(); }
};