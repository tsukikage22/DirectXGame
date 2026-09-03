/// @file ScenePass.h
/// @brief シーン描画パス

#pragma once

#include <d3d12.h>

#include "Engine/Core/ComPtr.h"

// 前方宣言
struct ScenePassBindings;
class GraphicsDevice;
class Scene;

class ScenePass
{
public:
    ScenePass()  = default;
    ~ScenePass() = default;

    /// @brief PSOとRSの構築
    /// @param device デバイス
    /// @return 成功した場合はtrue，失敗した場合はfalse
    bool Init(GraphicsDevice& device);

    /// @brief 終了処理
    void Term();

    /// @brief 描画コマンドの記録
    void Draw(const ScenePassBindings& passBindings, Scene& scene);

private:
    // ルートシグネチャ内でのルートパラメータ番号
    // Addxxxの呼び出し順と一致させる
    enum RootParam
    {
        CBV_Scene       = 0,  // b0
        CBV_Transform   = 1,  // b1
        CBV_Material    = 2,  // b2
        CBV_Display     = 3,  // b3
        SRV_Texture     = 4,  // t0-t4
        SRV_IESProfile  = 5,  // t0, space1
        SRV_Lights      = 6,  // t0, space2
        SRV_Irradiance  = 7,  // t0, space3
        SRV_Prefiltered = 8,  // t1, space3
        SRV_BrdfLut     = 9,  // t2, space3
        SRV_ShadowMap   = 10, // t0, space4
    };

    GraphicsDevice* m_pDevice = nullptr;

    engine::ComPtr<ID3D12RootSignature> m_pRootSignature; // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;           // パイプラインステート
};
