/// @file RenderTargetLayout.h
/// @brief RenderTargetLayout構造体とSetRenderTargets関数の定義

#pragma once

#include <d3d12.h>

#include <array>
#include <cassert>

#include "Engine/Core/EngineConfig.h"

/// @brief レンダーターゲットのレイアウト
struct RenderTargetLayout {
    // RTとDSのフォーマット，サンプル数は複数の場所で使用するため，ここでまとめて管理する
    std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT>
        rtvFormats{};
    UINT numRenderTargets = 1;                    // RTの数
    DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;  // DSVのフォーマット
    UINT sampleCount      = 1;                    // サンプル数
};

// シーン描画用：HDRバッファ＋深度
inline constexpr RenderTargetLayout kSceneLayout = {
    { config::kBackBufferFormat },  // RTフォーマット
    1,                              // RTの数
    config::kDepthBufferFormat,     // DSVフォーマット
    1                               // サンプル数
};

// ImGui用オフスクリーンパス：ガンマ空間＋深度なし
inline constexpr RenderTargetLayout kImGuiLayout = {
    { config::kUIBufferFormat },  // RTフォーマット
    1,                            // RTの数
    DXGI_FORMAT_UNKNOWN,          // DSVフォーマット
    1                             // サンプル数
};

// 最終合成パス：scRGB＋深度なし
inline constexpr RenderTargetLayout kCompositeLayout = {
    { config::kBackBufferFormat },  // RTフォーマット
    1,                              // RTの数
    DXGI_FORMAT_UNKNOWN,            // DSVフォーマット
    1                               // サンプル数
};

/// @brief RTLayout定数からSetRenderTargetsを呼び出す
/// @param pCmdList コマンドリスト
/// @param layout RTLayout定数
/// @param pRTVHandles RTVハンドルの配列
/// @param pDSVHandle DSVハンドル
inline void SetRenderTargets(ID3D12GraphicsCommandList* pCmdList,
    const RenderTargetLayout& layout,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle) {
    // DSVフォーマットが設定されている場合はDSVハンドルがnullptrでないことを確認
    assert((layout.dsvFormat == DXGI_FORMAT_UNKNOWN || pDSVHandle != nullptr) &&
           "DSV handle must be provided if DSV format is set.");

    // レンダーターゲットと深度ステンシルを設定
    pCmdList->OMSetRenderTargets(
        layout.numRenderTargets, pRTVHandles, FALSE, pDSVHandle);
}
