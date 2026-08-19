/// @file Renderer.h
/// @brief レンダリングに関する処理を行うクラス

#pragma once

#include <cstdint>

#include "Engine/Core/SwapChain.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/DepthTarget.h"

class GraphicsDevice;

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

    //==========================================================
    // アクセサ
    //==========================================================
    /// @brief スワップチェインの取得
    SwapChain& GetSwapChain() { return m_swapChain; }

    /// @brief 深度バッファの取得
    DepthTarget& GetDepthBuffer() { return m_depthTarget; }

    /// @brief UI用レンダーターゲットの取得
    ColorTarget& GetUITarget() { return m_uiTarget; }

    /// @brief 現在のフレーム番号を取得
    uint32_t GetFrameIndex() const { return m_swapChain.GetFrameIndex(); }

private:
    SwapChain m_swapChain;      // スワップチェイン
    DepthTarget m_depthTarget;  // 深度バッファ
    ColorTarget m_uiTarget;     // UI用レンダーターゲット

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};