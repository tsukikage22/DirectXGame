#include "Engine/RootSignature.h"

bool RootSignature::Create(ID3D12Device* pDevice, RootSignatureBuilder& builder,
    RootSignature** ppRootSignature) {
    if (ppRootSignature) {
        *ppRootSignature = this;
    }

    builder.Build(pDevice, m_pRootSignature.GetAddressOf());
    return m_pRootSignature ? true : false;
}