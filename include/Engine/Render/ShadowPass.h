/// @file ShadowPass.h
/// @brief シャドウマップの描画パス

#pragma once

#include <d3d12.h>

#include "Engine/Core/ComPtr.h"

class GraphicsDevice;
class Scene;
struct ShadowPassBindings;

class ShadowPass
{
public:
    ShadowPass()  = default;
    ~ShadowPass() = default;

    /// @brief PSOとRSの構築
    /// @param device デバイス
    /// @return 成功した場合はtrue，失敗した場合はfalse
    bool Init(GraphicsDevice& device);

    void Term();

    /// @brief 描画コマンドの記録
    /// @param passBindings パスバインディング
    void Draw(const ShadowPassBindings& passBindings, Scene& scene);

private:
    enum RootParam
    {
        CBV_Scene     = 0, // b0
        CBV_Transform = 1, // b1
    };

    GraphicsDevice* m_pDevice = nullptr;

    engine::ComPtr<ID3D12RootSignature> m_pRootSignature; // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;           // パイプラインステート
};
