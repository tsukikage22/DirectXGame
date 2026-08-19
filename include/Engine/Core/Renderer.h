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

    /// @brief スワップチェインの取得
    SwapChain& GetSwapChain() { return m_swapChain; }

    /// @brief 深度バッファの取得
    DepthTarget& GetDepthBuffer() { return m_depthTarget; }

    /// @brief UI用レンダーターゲットの取得
    ColorTarget& GetUITarget() { return m_uiTarget; }

private:
    SwapChain m_swapChain;      // スワップチェイン
    DepthTarget m_depthTarget;  // 深度バッファ
    ColorTarget m_uiTarget;     // UI用レンダーターゲット

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};