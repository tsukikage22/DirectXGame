/// @file   TransformGPU.h
/// @brief  ワールド行列の定数バッファ作成
#pragma once

#include <DirectXMath.h>

#include "Engine/ConstantBuffer.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ShaderConstants.h"

class TransformGPU {
public:
    TransformGPU();
    ~TransformGPU();

    /// @brief 初期化処理，定数バッファの作成
    /// @param pDevice デバイス
    /// @param pPoolCBV 定数バッファ用のディスクリプタプール
    /// @return 成功したらtrue
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        const DirectX::XMMATRIX& world = DirectX::XMMatrixIdentity());

    /// @brief 終了処理，リソースの解放
    void Term();

    /// @brief ワールド行列の更新
    /// @param world ワールド行列
    void Update(const DirectX::XMMATRIX& world);

    /// @brief GPUハンドルの取得
    /// @return GPUハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const {
        return m_constantBuffer.GetGPUHandle();
    }

private:
    ConstantBuffer m_constantBuffer;         // ワールド行列用の定数バッファ
    shader::TransformConstants m_constants;  // 定数バッファ用データ

    // コピー禁止
    TransformGPU(const TransformGPU&)            = delete;
    TransformGPU& operator=(const TransformGPU&) = delete;
};