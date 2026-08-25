#include "Engine/Graphics/ComputePipelineBuilder.h"

#include <cassert>

#include "Engine/Core/DxDebug.h"

ComputePipelineBuilder& ComputePipelineBuilder::SetRootSignature(
    ID3D12RootSignature* pRootSignature) {
    m_PSOdesc.pRootSignature = pRootSignature;

    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::SetComputeShader(
    const std::byte* pShaderBytecode, std::size_t bytecodeLength) {
    m_PSOdesc.CS.pShaderBytecode = pShaderBytecode;
    m_PSOdesc.CS.BytecodeLength  = bytecodeLength;

    return *this;
}

bool ComputePipelineBuilder::Build(ID3D12Device* pDevice) {
    if (!pDevice) {
        return false;
    }

    assert(m_PSOdesc.pRootSignature != nullptr && "RootSignature is not set.");
    assert(
        m_PSOdesc.CS.pShaderBytecode != nullptr && "ComputeShader is not set.");

    CHECK_HR(pDevice, pDevice->CreateComputePipelineState(&m_PSOdesc,
                          IID_PPV_ARGS(m_pPSO.ReleaseAndGetAddressOf())));
    return true;
}

void ComputePipelineBuilder::SetDefault() {
    m_PSOdesc.Flags    = D3D12_PIPELINE_STATE_FLAG_NONE;
    m_PSOdesc.NodeMask = 0;
}