#pragma once

#include <d3d12.h>

#include "Engine/ComPtr.h"
#include "Engine/SceneConstantsGPU.h"
#include "Engine/TransformGPU.h"

class FrameResource {
public:
    FrameResource();
    ~FrameResource();

    /// @brief 初期化
    /// @return
    bool Init(
        ID3D12Device* pDevice, DescriptorPool* pPoolCBV, size_t numObjects);

    void Term();

    void BeginFrame(ID3D12GraphicsCommandList* pCmdList);

    void EndFrame(UINT64 fenceValue);

    //=======================================
    // アクセサ
    //=======================================

    /// @brief コマンドアロケータの取得
    /// @return
    ID3D12CommandAllocator* GetCommandAllocator() const {
        return m_pCmdAllocator.Get();
    }

    /// @brief TransformGPUの取得
    /// @return
    std::vector<TransformGPU>& GetTransforms() { return m_transforms; }

    /// @brief SceneConstantsGPUの取得
    /// @return
    SceneConstantsGPU& GetSceneConstants() { return m_sceneConstants; }

    /// @brief フェンス値の取得
    /// @return
    UINT64 GetFenceValue() const { return m_fenceValue; }

private:
    engine::ComPtr<ID3D12CommandAllocator>
        m_pCmdAllocator;                     // コマンドアロケータ
    std::vector<TransformGPU> m_transforms;  // 各オブジェクトのワールド行列
    SceneConstantsGPU m_sceneConstants;      // シーン定数
    UINT64 m_fenceValue = 0;                 // フェンス値

    // コピー禁止
    FrameResource(const FrameResource&)            = delete;
    FrameResource& operator=(const FrameResource&) = delete;
};
