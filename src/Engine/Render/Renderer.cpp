#include "Engine/Render/Renderer.h"

#include <Windows.h>

#include <array>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/RenderTargetLayout.h"
#include "Engine/Resource/AssetSystem.h"
#include "Engine/Scene/Scene.h"
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

    m_pDevice = &device;

    // スワップチェインの生成
    if (!m_swapChain.Init(device, width, height, hWnd)) {
        return false;
    }

    // 深度バッファの生成
    // 将来的にシーン描画パスのターゲットがバックバッファと一致しなくなった場合は，
    // ここで幅と高さをシーン描画パスのRTの幅と高さに合わせる必要がある
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

    // フレームリソースの初期化
    for (int i = 0; i < config::kFrameCount; i++) {
        if (!m_frameResources[i].Init(device)) {
            return false;
        }
    }

    // コマンドリストの生成
    CHECK_HR(device.GetDevice(),
        device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_frameResources[GetFrameIndex()].GetCommandAllocator(), nullptr,
            IID_PPV_ARGS(m_pCmdList.GetAddressOf())));
    m_pCmdList->Close();

    return true;
}

void Renderer::Term() {
    m_pDevice = nullptr;

    // ディスプレイCBの破棄
    m_displayConstantsGPU.Term();

    // UI用レンダーターゲットの終了処理
    m_uiTarget.Term();

    // 深度バッファの終了処理
    m_depthTarget.Term();

    // スワップチェインの終了処理
    m_swapChain.Term();

    // フレームリソースの解放
    for (int i = 0; i < config::kFrameCount; i++) {
        m_frameResources[i].Term();
    }

    // コマンドリストの解放
    m_pCmdList.Reset();
}

void Renderer::BeginFrame() {
    // フレームレイテンシ待機
    WaitFrameLatency();

    // フェンス同期
    uint32_t frameIndex = GetFrameIndex();
    uint64_t fenceValue = m_frameResources[frameIndex].GetFenceValue();
    // 初回フレーム（fencevalue == 0）の場合は待機をスキップ
    if (fenceValue != 0) {
        m_pDevice->GetCommandQueue().Wait(fenceValue, INFINITE);
    }

    // コマンドリスト/アロケータのリセット
    m_frameResources[frameIndex].BeginFrame(m_pCmdList.Get());

    // リソースバリア(Present -> RenderTarget)の設定
    D3D12_RESOURCE_BARRIER barrier =
        MakeTransitionBarrier(m_swapChain.GetBackBuffer().GetResource(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCmdList->ResourceBarrier(1, &barrier);
}

void Renderer::BeginScenePass() {
    // バックバッファの取得
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();

    // レンダーターゲットの設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = backBuffer.GetRTVCPUHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthTarget.GetCPUHandle();
    SetRenderTargets(m_pCmdList.Get(), kSceneLayout, &rtvHandle, &dsvHandle);

    // レンダーターゲットのクリア
    const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    m_pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pCmdList->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定
    auto viewport    = backBuffer.MakeViewport();
    auto scissorRect = backBuffer.MakeScissorRect();
    m_pCmdList->RSSetViewports(1, &viewport);
    m_pCmdList->RSSetScissorRects(1, &scissorRect);
}

void Renderer::BeginCompositePass() {
    // バックバッファの取得
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();

    // レンダーターゲットの設定
    auto rtvHandle = backBuffer.GetRTVCPUHandle();
    SetRenderTargets(m_pCmdList.Get(), kCompositeLayout, &rtvHandle, nullptr);

    // ビューポートの設定
    auto viewport = backBuffer.MakeViewport();
    m_pCmdList->RSSetViewports(1, &viewport);
    auto scissorRect = backBuffer.MakeScissorRect();
    m_pCmdList->RSSetScissorRects(1, &scissorRect);
}

void Renderer::UpdateConstants(Scene& scene, uint32_t debugView) {
    uint32_t frameIndex          = GetFrameIndex();
    FrameResource& frameResource = m_frameResources[frameIndex];

    // 定数バッファの中身(行列やマテリアル情報)の更新
    // シーン内の全ゲームオブジェクトのtransformを更新
    scene.ForEachObject(
        [&](GameObject& obj) { obj.UpdateTransformGPU(frameIndex); });

    // シーン内ライトの更新
    std::array<shader::LightConstants, config::kMaxLights> lights = {};
    uint32_t count = 0;  // 実際にコピーされたライトの数
    scene.ForEachLight([&](Light& light) {
        if (!light.IsEnabled() || count >= config::kMaxLights) {
            return;
        }
        lights[count++] = light.ToShaderConstants();
    });
    uint32_t uploadedCount =  // バッファにコピーされたライトの数
        frameResource.GetLightBuffer().Update(lights.data(), count);

    // シーン定数の更新
    shader::SceneConstants sc{};

    // ビュー行列・射影行列を転置して格納
    Camera& camera                 = scene.GetCamera();
    DirectX::XMFLOAT4X4 view       = camera.GetViewMatrix();
    DirectX::XMFLOAT4X4 projection = camera.GetProjectionMatrix();
    DirectX::XMMATRIX viewMat      = DirectX::XMLoadFloat4x4(&view);
    DirectX::XMMATRIX projMat      = DirectX::XMLoadFloat4x4(&projection);
    DirectX::XMStoreFloat4x4(&sc.view, DirectX::XMMatrixTranspose(viewMat));
    DirectX::XMStoreFloat4x4(
        &sc.projection, DirectX::XMMatrixTranspose(projMat));

    // NDCからワールド座標への変換行列を計算して格納
    DirectX::XMMATRIX invViewProj =
        DirectX::XMMatrixMultiply(DirectX::XMMatrixInverse(nullptr, projMat),
            DirectX::XMMatrixInverse(nullptr, viewMat));
    DirectX::XMStoreFloat4x4(
        &sc.invViewProj, DirectX::XMMatrixTranspose(invViewProj));

    // カメラ位置・時間・ライト数・露出・デバッグビューの設定
    sc.cameraPosition = camera.GetTransform().GetPosition();
    sc.time           = static_cast<float>(GetTickCount64()) / 1000.0f;
    sc.lightCount     = uploadedCount;  // 実際にアップロードされたライトの数
    sc.exposure       = camera.ComputeExposure();
    sc.debugView      = debugView;
    sc.envIntensity   = scene.GetEnvIntensity();

    frameResource.GetSceneConstants().Update(sc);
}

void Renderer::EndFrame() {
    // 1. リソースバリアの設定（RT -> Present）
    D3D12_RESOURCE_BARRIER barrier =
        MakeTransitionBarrier(m_swapChain.GetBackBuffer().GetResource(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 2. コマンドリストのクローズ
    m_pCmdList->Close();

    // 3. コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCmdList.Get() };
    m_pDevice->GetCommandQueue().Execute(
        ppCommandLists, _countof(ppCommandLists));

    // 4. フェンスの発行
    UINT64 fenceValue = m_pDevice->GetCommandQueue().Signal();

    // 5. フェンス値の保存
    m_frameResources[m_swapChain.GetFrameIndex()].EndFrame(fenceValue);
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

bool Renderer::ResizeBuffers(uint32_t width, uint32_t height) {
    // サイズ確認
    ColorTarget& backBuffer = m_swapChain.GetBackBuffer();
    if (backBuffer.GetWidth() == width && backBuffer.GetHeight() == height) {
        return true;
    }

    // フェンス待機
    m_pDevice->WaitForGPU();

    // スワップチェインのリサイズ
    if (!m_swapChain.Resize(*m_pDevice, width, height)) {
        return false;
    }

    // 深度バッファのリサイズ（再生成）
    m_depthTarget.Term();
    if (!m_depthTarget.Init(m_pDevice->GetDevice(), m_pDevice->DsvPool(), width,
            height, config::kDepthBufferFormat)) {
        return false;
    }

    // UI用レンダーターゲットのリサイズ（再生成）
    m_uiTarget.Term();
    if (!m_uiTarget.Init(m_pDevice->GetDevice(), m_pDevice->RtvPool(),
            m_pDevice->CbvSrvUavPool(), width, height,
            config::kUIBufferFormat)) {
        return false;
    }

    return true;
}

ScenePassBindings Renderer::MakeScenePassBindings(AssetSystem& assetSystem) {
    uint32_t frameIndex          = GetFrameIndex();
    FrameResource& frameResource = m_frameResources[frameIndex];

    ScenePassBindings context = {};
    context.pCmdList          = m_pCmdList.Get();
    context.frameIndex        = frameIndex;
    context.pCbvSrvUavHeap    = m_pDevice->CbvSrvUavPool()->GetHeap();
    context.sceneCB   = frameResource.GetSceneConstants().GetGPUAddress();
    context.displayCB = m_displayConstantsGPU.GetGPUAddress();
    context.lightSRV  = frameResource.GetLightBuffer().GetGPUHandle();
    context.iesSRV    = assetSystem.GetIesSrvGpuHandle();

    assert(context.IsValid() && "ScenePassBindings is not valid.");

    return context;
}

CompositePassBindings Renderer::MakeCompositePassBindings() {
    CompositePassBindings context = {};
    context.pCmdList              = m_pCmdList.Get();
    context.pCbvSrvUavHeap        = m_pDevice->CbvSrvUavPool()->GetHeap();
    context.displayCB             = m_displayConstantsGPU.GetGPUAddress();
    context.uiSRV                 = m_uiTarget.GetSRVGPUHandle();

    assert(context.IsValid() && "CompositePassBindings is not valid.");

    return context;
}

SkyboxPassBindings Renderer::MakeSkyboxPassBindings(AssetSystem& assetSystem) {
    uint32_t frameIndex          = GetFrameIndex();
    FrameResource& frameResource = m_frameResources[frameIndex];

    SkyboxPassBindings context = {};
    context.pCmdList           = m_pCmdList.Get();
    context.pCbvSrvUavHeap     = m_pDevice->CbvSrvUavPool()->GetHeap();
    context.sceneCB   = frameResource.GetSceneConstants().GetGPUAddress();
    context.displayCB = m_displayConstantsGPU.GetGPUAddress();
    context.skyboxSRV = assetSystem.GetEnvMapCubemapSrvGpuHandle();

    assert(context.IsValid() && "SkyboxPassBindings is not valid.");

    return context;
}
