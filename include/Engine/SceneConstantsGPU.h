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
    /// @param pDevice
    /// @param pPoolCBV
    /// @param sceneConstants
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        const shader::SceneConstants& sceneConstants);

    /// @brief リソースの解放
    void Term();

    /// @brief 定数バッファの更新
    /// @param sceneConstants
    void Update(const shader::SceneConstants& sceneConstants);

private:
    ConstantBuffer m_constantBuffer;     // シーン定数用の定数バッファ
    shader::SceneConstants m_constants;  // 定数バッファ用データ

    // コピー禁止
    SceneConstantsGPU(const SceneConstantsGPU&)            = delete;
    SceneConstantsGPU& operator=(const SceneConstantsGPU&) = delete;
};