/// @file Renderer.h
/// @brief レンダリングに関する処理を行うクラス

#pragma once

#include <cstdint>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/FrameResource.h"
#include "Engine/Core/SwapChain.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/DepthTarget.h"
#include "Engine/Shader/DisplayConstantsGPU.h"

class GraphicsDevice;

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
    /// @details スワップチェイン・深度バッファの生成
    /// スワップチェイン・深度バッファの生成
    /// UI用レンダーターゲットの生成
    /// ディスプレイCBの生成
    /// フレームリソースの生成
    /// コマンドリストの生成
    /// @param device
    /// @param width
    /// @param height
    /// @param hWnd
    /// @return
    bool Init(
        GraphicsDevice& device, uint32_t width, uint32_t height, HWND hWnd);
    void Term();

    /// @brief フレーム開始時の処理
    void BeginFrame();

    /// @brief UI合成パスの開始
    void BeginCompositePass();

    /// @brief  フレーム終了時の処理
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
    /// @param device グラフィックスデバイス
    /// @param width 幅
    /// @param height 高さ
    bool ResizeBuffers(
        GraphicsDevice& graphicsDevice, uint32_t width, uint32_t height);

    //==========================================================
    // アクセサ
    //==========================================================
    /// @brief 深度バッファ
    DepthTarget& GetDepthBuffer() { return m_depthTarget; }

    /// @brief UI用レンダーターゲット
    ColorTarget& GetUITarget() { return m_uiTarget; }

    /// @brief 現在のフレーム番号
    uint32_t GetFrameIndex() const { return m_swapChain.GetFrameIndex(); }

    /// @brief ディスプレイ情報
    DisplayInfo GetDisplayInfo() const { return m_displayInfo; }

    /// @brief ディスプレイCBのGPUアドレス
    D3D12_GPU_VIRTUAL_ADDRESS GetDisplayConstantsAddress() const {
        return m_displayConstantsGPU.GetGPUAddress();
    }

    /// @brief フレームリソース
    FrameResource& GetFrameResource() {
        return m_frameResources[m_swapChain.GetFrameIndex()];
    }

    /// @brief コマンドリスト
    ID3D12GraphicsCommandList* GetCommandList() { return m_pCmdList.Get(); }

private:
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト

    SwapChain m_swapChain;      // スワップチェイン
    DepthTarget m_depthTarget;  // 深度バッファ
    ColorTarget m_uiTarget;     // UI用レンダーターゲット

    FrameResource m_frameResources[config::kFrameCount];  // フレームリソース

    DisplayInfo m_displayInfo = {};             // ディスプレイ情報
    DisplayConstantsGPU m_displayConstantsGPU;  // ディスプレイCB
    HWND m_hWnd = nullptr;                      // ウィンドウハンドル

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};