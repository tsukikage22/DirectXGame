/// @file Renderer.h
/// @brief レンダリングに関する処理を行うクラス

#pragma once

#include <cstdint>

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

    bool Init(
        GraphicsDevice& device, uint32_t width, uint32_t height, HWND hWnd);
    void Term();

    /// @brief フレーム開始時の処理
    /// @param pCmdList コマンドリスト
    void BeginFrame(ID3D12GraphicsCommandList* pCmdList);

    /// @brief UI合成パスの開始
    void BeginCompositePass(ID3D12GraphicsCommandList* pCmdList);

    /// @brief  フレーム終了時の処理
    /// @param pCmdList コマンドリスト
    void EndFrame(ID3D12GraphicsCommandList* pCmdList);

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

private:
    SwapChain m_swapChain;      // スワップチェイン
    DepthTarget m_depthTarget;  // 深度バッファ
    ColorTarget m_uiTarget;     // UI用レンダーターゲット

    DisplayInfo m_displayInfo = {};             // ディスプレイ情報
    DisplayConstantsGPU m_displayConstantsGPU;  // ディスプレイCB
    HWND m_hWnd = nullptr;                      // ウィンドウハンドル

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};