/// @file SwapChain.h
/// @brief スワップチェインの生成と保持，及びPresentやResizeの処理を行う

#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/Core/ComPtr.h"

class GraphicsDevice;

class SwapChain {
public:
    SwapChain()  = default;
    ~SwapChain() = default;

    /// @brief スワップチェインの生成
    /// @param graphicsDevice GraphicsDeviceオブジェクト
    /// @param format バックバッファのフォーマット
    /// @param width バックバッファの幅
    /// @param height バックバッファの高さ
    /// @param hWnd ウィンドウハンドル
    /// @return 初期化に成功したかどうか
    bool Init(GraphicsDevice& graphicsDevice, DXGI_FORMAT format,
        uint32_t width, uint32_t height, HWND hWnd);

    /// @brief 終了処理
    void Term();

    /// @brief 画面表示
    void Present();

    /// @brief フレームレイテンシ待機オブジェクトを待機する
    /// @param timeout タイムアウト時間（ミリ秒）
    void WaitForFrameLatency(DWORD timeout = 1000) {
        WaitForSingleObject(m_frameLatencyWaitableObject, timeout);
    }

    /// @brief スワップチェインの取得
    /// @return
    IDXGISwapChain3* GetSwapChain() { return m_pSwapChain.Get(); }

    /// @brief 現在のフレーム番号を取得
    uint32_t GetFrameIndex() const { return m_frameIndex; }

private:
    engine::ComPtr<IDXGISwapChain3> m_pSwapChain;  // スワップチェイン
    uint32_t m_frameIndex = 0;                     // 現在のフレーム番号
    HANDLE m_frameLatencyWaitableObject =
        nullptr;  // フレームレイテンシ待機オブジェクト

    // コピー禁止
    SwapChain(const SwapChain&)            = delete;
    SwapChain& operator=(const SwapChain&) = delete;
};