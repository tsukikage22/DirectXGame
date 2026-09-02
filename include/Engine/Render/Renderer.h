/// @file Renderer.h
/// @brief レンダリングに関する処理を行うクラス

#pragma once

#include <cstdint>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/FrameResource.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/DepthTarget.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Render/SwapChain.h"
#include "Engine/Shader/DisplayConstantsGPU.h"

// 前方宣言
class GraphicsDevice;
class Scene;
class AssetSystem;

/// @brief ディスプレイ情報
struct DisplayInfo {
    HMONITOR hMonitor;
    bool isHDRSupported;
    float maxLuminance;
    float minLuminance;
    float maxFullFrameLuminance;
};

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    /// @brief レンダラーの初期化
    /// @param device デバイス
    /// @param width 幅
    /// @param height 高さ
    /// @param hWnd ウィンドウハンドル
    bool Init(
        GraphicsDevice& device, uint32_t width, uint32_t height, HWND hWnd);
    void Term();

    /// @brief
    /// フレームレイテンシ待機，フェンス同期，コマンドリストのリセット，リソースバリア遷移
    void BeginFrame();

    /// @brief シーン描画パスの開始
    void BeginScenePass();

    /// @brief UI合成パスの開始
    void BeginCompositePass();

    /// @brief 定数バッファの更新
    /// @param scene シーン
    /// @param debugView デバッグビュー
    void UpdateConstants(Scene& scene, uint32_t debugView);

    /// @brief フレーム終了時の処理
    void EndFrame();

    /// @brief 画面表示
    void Present() { m_swapChain.Present(); }

    /// @brief フレームレイテンシの待機
    /// @param timeout 待機時間（ミリ秒）
    void WaitFrameLatency(DWORD timeout = 1000) {
        m_swapChain.WaitForFrameLatency(timeout);
    }

    /// @brief モニター変更を検出する
    bool DetectMonitorChange();

    /// @brief ディスプレイ情報の更新
    void QueryDisplayInfo();

    /// @brief ディスプレイCBの更新
    void UploadDisplayConstants();

    /// @brief バックバッファと深度バッファ，UI用RTのリサイズ
    /// @param width 幅
    /// @param height 高さ
    bool ResizeBuffers(uint32_t width, uint32_t height);

    /// @brief シーン描画パスに渡す情報をまとめた構造体を作成する
    ScenePassBindings MakeScenePassBindings(AssetSystem& assetSystem);

    /// @brief 合成パスに渡す情報をまとめた構造体を作成する
    CompositePassBindings MakeCompositePassBindings();

    /// @brief スカイボックス描画パスに渡す情報をまとめた構造体を作成する
    SkyboxPassBindings MakeSkyboxPassBindings(AssetSystem& assetSystem);

    //==========================================================
    // アクセサ
    //==========================================================
    /// @brief UI用レンダーターゲット
    ColorTarget& GetUITarget() { return m_uiTarget; }

    /// @brief 現在のフレーム番号
    uint32_t GetFrameIndex() const { return m_swapChain.GetFrameIndex(); }

    /// @brief コマンドリスト
    ID3D12GraphicsCommandList* GetCommandList() { return m_pCmdList.Get(); }

private:
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト

    GraphicsDevice* m_pDevice = nullptr;  // グラフィックスデバイス
    SwapChain m_swapChain;                // スワップチェイン
    DepthTarget m_depthTarget;            // 深度バッファ
    DepthTarget m_shadowMap;              // シャドウマップ
    ColorTarget m_uiTarget;               // UI用レンダーターゲット

    FrameResource m_frameResources[config::kFrameCount];  // フレームリソース

    DisplayInfo m_displayInfo = {};             // ディスプレイ情報
    DisplayConstantsGPU m_displayConstantsGPU;  // ディスプレイCB
    HWND m_hWnd = nullptr;                      // ウィンドウハンドル

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};