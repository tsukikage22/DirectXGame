#pragma once

#include <d3d12.h>

#include <memory>
#include <vector>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DescriptorPool.h"
#include "Engine/Shader/LightBuffer.h"
#include "Engine/Shader/LightingConstantsGPU.h"
#include "Engine/Shader/SceneConstantsGPU.h"
#include "Engine/Shader/TransformGPU.h"

class FrameResource {
public:
    FrameResource();
    ~FrameResource();

    /// @brief 初期化
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    /// @brief 終了処理
    void Term();

    /// @brief フレーム開始
    /// @param pCmdList
    void BeginFrame(ID3D12GraphicsCommandList* pCmdList);

    /// @brief フレーム終了
    /// @param fenceValue
    void EndFrame(UINT64 fenceValue);

    //=======================================
    // アクセサ
    //=======================================

    /// @brief コマンドアロケータの取得
    ID3D12CommandAllocator* GetCommandAllocator() const {
        return m_pCmdAllocator.Get();
    }

    /// @brief SceneConstantsGPUの取得
    SceneConstantsGPU& GetSceneConstants() { return m_sceneConstants; }

    /// @brief LightingConstantsGPUの取得
    LightingConstantsGPU& GetLightingConstants() { return m_lightingConstants; }

    /// @brief LightBufferの取得
    LightBuffer& GetLightBuffer() { return m_lightBuffer; }

    /// @brief フェンス値の取得
    UINT64 GetFenceValue() const { return m_fenceValue; }

private:
    engine::ComPtr<ID3D12CommandAllocator>
        m_pCmdAllocator;  // コマンドアロケータ

    SceneConstantsGPU m_sceneConstants;        // シーン定数
    LightingConstantsGPU m_lightingConstants;  // ライティング定数
    LightBuffer m_lightBuffer;                 // ライトバッファ

    UINT64 m_fenceValue = 0;  // フェンス値
    // このフレームを作成した時点のフェンス値を持つことで，
    // 再利用の判断や寿命の管理に使用できる

    bool m_isActive = false;  // フレームリソースが使用中かどうか

    // コピー禁止
    FrameResource(const FrameResource&)            = delete;
    FrameResource& operator=(const FrameResource&) = delete;
};
