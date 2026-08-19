#include "Engine/Core/Renderer.h"

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"

namespace /* anonymous */ {
/// @brief リソースバリアの作成
D3D12_RESOURCE_BARRIER MakeTransitionBarrier(ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter  = after;
    return barrier;
}
}  // namespace

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

void Renderer::BeginFrame(ID3D12GraphicsCommandList* pCmdList) {
    // バックバッファの取得
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();

    // リソースバリア(Present -> RenderTarget)の設定
    D3D12_RESOURCE_BARRIER barrier =
        MakeTransitionBarrier(backBuffer.GetResource(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCmdList->ResourceBarrier(1, &barrier);

    // レンダーターゲットの設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = backBuffer.GetRTVCPUHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthTarget.GetCPUHandle();
    BeginPass(pCmdList, kGeometryLayout, &rtvHandle, &dsvHandle);

    // レンダーターゲットのクリア
    const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    pCmdList->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定
    auto viewport    = backBuffer.MakeViewport();
    auto scissorRect = backBuffer.MakeScissorRect();
    pCmdList->RSSetViewports(1, &viewport);
    pCmdList->RSSetScissorRects(1, &scissorRect);
}

void Renderer::BeginCompositePass(ID3D12GraphicsCommandList* pCmdList) {
    // バックバッファの取得
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();

    // レンダーターゲットの設定
    auto rtvHandle = backBuffer.GetRTVCPUHandle();
    BeginPass(pCmdList, kCompositeLayout, &rtvHandle, nullptr);

    // ビューポートの設定
    auto viewport = backBuffer.MakeViewport();
    pCmdList->RSSetViewports(1, &viewport);
    auto scissorRect = backBuffer.MakeScissorRect();
    pCmdList->RSSetScissorRects(1, &scissorRect);
}

void Renderer::EndFrame(ID3D12GraphicsCommandList* pCmdList) {
    // リソースバリアの設定（RT -> Present）
    D3D12_RESOURCE_BARRIER barrier =
        MakeTransitionBarrier(m_swapChain.GetBackBuffer().GetResource(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCmdList->ResourceBarrier(1, &barrier);
}
