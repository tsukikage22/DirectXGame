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
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV);
    // TODO: Transformの数を動的に変更できるようにする

    /// @brief 終了処理
    void Term();

    /// @brief Transformを追加
    bool AddTransform(
        ID3D12Device* pDevice, DescriptorPool* pPoolCBV, size_t count);

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
    // このフレームを作成した時点のフェンス値を持つことで，
    // 再利用の判断や寿命の管理に使用できる

    bool m_isActive = false;  // フレームリソースが使用中かどうか

    // コピー禁止
    FrameResource(const FrameResource&)            = delete;
    FrameResource& operator=(const FrameResource&) = delete;
};
