#pragma once

#include <d3d12.h>

#include <string>
#include <vector>

#include "Engine/ComPtr.h"
#include "Engine/RootSignatureBuilder.h"

class RootSignature {
public:
    RootSignature()  = default;
    ~RootSignature() = default;

    // ルートシグニチャの生成
    bool Create(ID3D12Device* pDevice, RootSignatureBuilder& builder,
        RootSignature** ppRootSignature = nullptr);

    // ルートシグニチャの取得
    ID3D12RootSignature* Get() const { return m_pRootSignature.Get(); }

private:
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグニチャ
};