#include "Engine/Render/SwapChain.h"

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"

bool SwapChain::Init(GraphicsDevice& graphicsDevice, uint32_t width,
    uint32_t height, HWND hWnd) {
    // スワップチェインの設定
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width                 = width;
    desc.Height                = height;
    desc.Format                = config::kBackBufferFormat;
    desc.Stereo                = FALSE;
    desc.SampleDesc.Count      = 1;
    desc.SampleDesc.Quality    = 0;
    desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount           = config::kFrameCount;
    desc.Scaling               = DXGI_SCALING_STRETCH;
    desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    // スワップチェインの生成
    engine::ComPtr<IDXGISwapChain1> pSwapChain;
    CHECK_HR(graphicsDevice.GetDevice(),
        graphicsDevice.GetFactory()->CreateSwapChainForHwnd(
            graphicsDevice.GetCommandQueue().GetD3DQueue(), hWnd, &desc,
            nullptr, nullptr, pSwapChain.GetAddressOf()));

    // IDXGISwapChain3を取得
    CHECK_HR(graphicsDevice.GetDevice(), pSwapChain.As(&m_pSwapChain));

    // バックバッファ番号を取得
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    // カラースペースの設定（scRGB対応）
    m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);

    // フレームレイテンシ待機オブジェクトの取得
    m_pSwapChain->SetMaximumFrameLatency(config::kFrameCount - 1);
    m_frameLatencyWaitableObject =
        m_pSwapChain->GetFrameLatencyWaitableObject();

    pSwapChain.Reset();

    // バックバッファの生成
    for (auto i = 0u; i < config::kFrameCount; i++) {
        if (!m_colorTarget[i].Init(graphicsDevice.GetDevice(),
                graphicsDevice.RtvPool(), i, m_pSwapChain.Get())) {
            return false;
        }
    }

    return true;
}

void SwapChain::Term() {
    // フレームレイテンシ待機オブジェクト
    if (m_frameLatencyWaitableObject) {
        CloseHandle(m_frameLatencyWaitableObject);
        m_frameLatencyWaitableObject = nullptr;
    }

    // バックバッファの解放
    for (auto i = 0u; i < config::kFrameCount; i++) {
        m_colorTarget[i].Term();
    }

    // スワップチェインの破棄
    m_pSwapChain.Reset();
}

void SwapChain::Present() {
    // スワップチェインのPresent
    m_pSwapChain->Present(1, 0);

    // バックバッファ番号を更新
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

bool SwapChain::Resize(
    GraphicsDevice& graphicsDevice, uint32_t width, uint32_t height) {
    // バックバッファの解放
    for (auto i = 0u; i < config::kFrameCount; i++) {
        m_colorTarget[i].Term();
    }

    // スワップチェインのリサイズ
    auto hr = m_pSwapChain.Get()->ResizeBuffers(config::kFrameCount, width,
        height, config::kBackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to resize swap chain buffers.\n");
        return false;
    }

    // バックバッファの再生成
    for (auto i = 0u; i < config::kFrameCount; i++) {
        if (!m_colorTarget[i].Init(graphicsDevice.GetDevice(),
                graphicsDevice.RtvPool(), i, m_pSwapChain.Get())) {
            return false;
        }
    }

    // frameIndexの更新
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    return true;
}