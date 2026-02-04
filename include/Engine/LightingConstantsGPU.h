/// @file LightingConstantsGPU.h
/// @brief ライティング定数のバッファ作成
#pragma once

#include <DirectXMath.h>

#include "Engine/ConstantBuffer.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ShaderConstants.h"

class LightingConstantsGPU {
public:
    LightingConstantsGPU();
    ~LightingConstantsGPU();

    /// @brief 定数バッファの作成
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        const shader::LightingConstants& lightingConstants);

    /// @brief 定数バッファの作成，初期値なし
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    void Term();

    void Update(const shader::LightingConstants& lightingConstants);

    //========================================
    // アクセサ
    //========================================

    /// @brief 定数バッファの取得
    ConstantBuffer& GetConstantBuffer() { return m_constantBuffer; }

    /// @brief 定数バッファのGPU仮想アドレスの取得
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const {
        return m_constantBuffer.GetGPUVirtualAddress();
    }

private:
    ConstantBuffer m_constantBuffer;        // ライティング定数用の定数バッファ
    shader::LightingConstants m_constants;  // 定数バッファ用データ

    // コピー禁止
    LightingConstantsGPU(const LightingConstantsGPU&)            = delete;
    LightingConstantsGPU& operator=(const LightingConstantsGPU&) = delete;
};