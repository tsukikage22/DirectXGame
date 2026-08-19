#include "Engine/Core/Renderer.h"

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GraphicsDevice.h"

bool Renderer::Init(
    GraphicsDevice& device, uint32_t width, uint32_t height, HWND hWnd) {
    // スワップチェインの生成
    if (!m_swapChain.Init(device, width, height, hWnd)) {
        return false;
    }

    // 深度バッファの生成
    // 将来的にジオメトリパスのターゲットがバックバッファと一致しなくなった場合は，
    // ここで幅と高さをジオメトリパスのRTの幅と高さに合わせる必要がある
    if (!m_depthTarget.Init(device.GetDevice(), device.DsvPool(), width, height,
            config::kDepthBufferFormat)) {
        return false;
    }

    // UI用レンダーターゲットの作成
    // UIは常に表示解像度（バックバッファに合わせる）
    if (!m_uiTarget.Init(device.GetDevice(), device.RtvPool(),
            device.CbvSrvUavPool(), width, height, config::kUIBufferFormat)) {
        return false;
    }
    return true;
}

void Renderer::Term() {
    // UI用レンダーターゲットの終了処理
    m_uiTarget.Term();

    // 深度バッファの終了処理
    m_depthTarget.Term();

    // スワップチェインの終了処理
    m_swapChain.Term();
}
