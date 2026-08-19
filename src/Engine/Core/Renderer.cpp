#include "Engine/Core/Renderer.h"

#include <Windows.h>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Shader/ShaderConstants.h"

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

/// @brief DisplayInfoからDisplayConstantsを作成する
shader::DisplayConstants MakeDisplayConstants(const DisplayInfo& info) {
    shader::DisplayConstants dc = {};
    dc.maxLuminance             = info.maxLuminance;
    dc.minLuminance             = info.minLuminance;
    dc.paperWhiteNits        = info.isHDRSupported ? config::kHDRPaperWhiteNits
                                                   : config::kSDRPaperWhiteNits;
    dc.maxFullFrameLuminance = info.maxFullFrameLuminance;
    return dc;
}

}  // namespace

bool Renderer::Init(
    GraphicsDevice& device, uint32_t width, uint32_t height, HWND hWnd) {
    m_hWnd = hWnd;

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

    // ディスプレイCBの作成
    if (!m_displayConstantsGPU.Init(
            device.GetDevice(), device.CbvSrvUavPool())) {
        return false;
    }

    // ディスプレイ情報とCBの更新
    QueryDisplayInfo();
    UploadDisplayConstants();

    return true;
}

void Renderer::Term() {
    // ディスプレイCBの破棄
    m_displayConstantsGPU.Term();

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

// モニター変更の検出
bool Renderer::DetectMonitorChange() {
    // 現在のモニターを取得
    HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);

    return (hMonitor != m_displayInfo.hMonitor);
}

// ディスプレイ情報の更新
void Renderer::QueryDisplayInfo() {
    // 出力情報の初期化
    DisplayInfo info           = {};
    info.isHDRSupported        = false;
    info.maxLuminance          = 80.0f;
    info.minLuminance          = 0.0f;
    info.maxFullFrameLuminance = 80.0f;

    // スワップチェーンから現座表示されているOutputを取得
    engine::ComPtr<IDXGIOutput> output;
    if (FAILED(m_swapChain.GetSwapChain()->GetContainingOutput(
            output.GetAddressOf()))) {
        m_displayInfo = info;
        return;
    };
    engine::ComPtr<IDXGIOutput6> output6;
    if (FAILED(output.As(&output6))) {
        m_displayInfo = info;
        return;
    }

    // ディスプレイの詳細情報を取得
    DXGI_OUTPUT_DESC1 desc1 = {};
    if (FAILED(output6->GetDesc1(&desc1))) {
        m_displayInfo = info;
        return;
    }

    info.hMonitor = desc1.Monitor;

    // HDR10対応チェック
    info.isHDRSupported =
        (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);

    if (info.isHDRSupported) {
        info.maxLuminance          = desc1.MaxLuminance;
        info.minLuminance          = desc1.MinLuminance;
        info.maxFullFrameLuminance = desc1.MaxFullFrameLuminance;
    }

    m_displayInfo = info;
}

// ディスプレイCBの更新
void Renderer::UploadDisplayConstants() {
    // ディスプレイ定数の作成
    shader::DisplayConstants dc = MakeDisplayConstants(m_displayInfo);

    // ディスプレイCBの更新
    m_displayConstantsGPU.Update(dc);
}

bool Renderer::ResizeBuffers(
    GraphicsDevice& graphicsDevice, uint32_t width, uint32_t height) {
    // サイズ確認
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();
    if (backBuffer.GetWidth() == width && backBuffer.GetHeight() == height) {
        return true;
    }

    // フェンス待機
    graphicsDevice.WaitForGPU();

    // スワップチェインのリサイズ
    if (!m_swapChain.Resize(graphicsDevice, width, height)) {
        return false;
    }

    // 深度バッファのリサイズ（再生成）
    m_depthTarget.Term();
    if (!m_depthTarget.Init(graphicsDevice.GetDevice(),
            graphicsDevice.DsvPool(), width, height,
            config::kDepthBufferFormat)) {
        return false;
    }

    // UI用レンダーターゲットのリサイズ（再生成）
    m_uiTarget.Term();
    if (!m_uiTarget.Init(graphicsDevice.GetDevice(), graphicsDevice.RtvPool(),
            graphicsDevice.CbvSrvUavPool(), width, height,
            config::kUIBufferFormat)) {
        return false;
    }

    return true;
}
