/// @file   SceneConstantsGPU.h
/// @brief  シーン定数のバッファ作成
#pragma once

#include <DirectXMath.h>

#include "Engine/ConstantBuffer.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ShaderConstants.h"

class SceneConstantsGPU {
public:
    SceneConstantsGPU();
    ~SceneConstantsGPU();

    /// @brief 定数バッファの作成
    /// @param pDevice デバイス
    /// @param pPoolCBV 定数バッファのディスクリプタプール
    /// @param sceneConstants 初期値
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        const shader::SceneConstants& sceneConstants);

    /// @brief 定数バッファの作成（デフォルト値）
    /// @param pDevice
    /// @param pPoolCBV
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    /// @brief リソースの解放
    void Term();

    /// @brief 定数バッファの更新
    /// @param sceneConstants
    void Update(const shader::SceneConstants& sceneConstants);

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
    ConstantBuffer m_constantBuffer;     // シーン定数用の定数バッファ
    shader::SceneConstants m_constants;  // 定数バッファ用データ

    // コピー禁止
    SceneConstantsGPU(const SceneConstantsGPU&)            = delete;
    SceneConstantsGPU& operator=(const SceneConstantsGPU&) = delete;
};