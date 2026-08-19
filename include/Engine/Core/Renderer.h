/// @file Renderer.h
/// @brief

#pragma once

#include "Engine/Core/SwapChain.h"

class GraphicsDevice;

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    bool Init(
        GraphicsDevice& device, HWND hWnd, uint32_t width, uint32_t height);
    void Term();

    /// @brief スワップチェインの取得
    SwapChain& GetSwapChain() { return m_SwapChain; }

private:
    SwapChain m_SwapChain;

    // コピー禁止
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
};