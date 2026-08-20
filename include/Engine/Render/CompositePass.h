/// @file CompositePass.h
/// @brief UI合成パス

#pragma once

#include <d3d12.h>

#include "Engine/Core/ComPtr.h"

// 前方宣言
struct CompositePassBindings;
class GraphicsDevice;

class CompositePass {
public:
    CompositePass()  = default;
    ~CompositePass() = default;

    /// @brief PSOとRSの構築
    bool Init(GraphicsDevice& device, ID3DBlob* vsBlob, ID3DBlob* psBlob);

    /// @brief 終了処理
    void Term();

    /// @brief 描画コマンドの記録
    void Draw(const CompositePassBindings& passBindings);

private:
    // ルートシグネチャ内でのルートパラメータ番号
    // Addxxxの呼び出し順と一致させる
    enum RootParam {
        CBV_Display = 0,  // b3
        SRV_UI      = 1,  // t0
    };

    GraphicsDevice* m_pDevice = nullptr;

    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;  // パイプラインステート
};
