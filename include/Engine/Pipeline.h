/// @file Pipeline.h
/// @brief ルートシグニチャとPSO

#include <d3d12.h>

#include <vector>

#include "Engine/ComPtr.h"

class RootSignature {
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;

public:
    RootSignature() = default;
    ~RootSignature() = default;

    bool Create(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc);
    ID3D12RootSignature* Get() const { return m_pRootSignature.Get(); }
};

class PipelineState {
    engine::ComPtr<ID3D12PipelineState> m_pPipelineState;

public:
    PipelineState() = default;
    ~PipelineState() = default;

    bool Create(
        ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
    ID3D12PipelineState* Get() const { return m_pPipelineState.Get(); }
};