//////////////////////////////////////////
/// @file Engine.cpp
/// @brief
//////////////////////////////////////////

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#include "Engine/Engine.h"

#include <cstdint>

#include "Engine/Debug/DebugUI.h"
#include "Engine/Resource/AssetLoadScope.h"

///////////////////////////////////////////
// Linker
///////////////////////////////////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "DirectXTex.lib")

////////////////////////////////////////////
// Engine class
////////////////////////////////////////////

// 初期化
bool Engine::Initialize(HWND hWnd, uint32_t width, uint32_t height) {
    // D3D初期化
    if (!InitD3D(hWnd, width, height)) {
        MessageBoxW(
            nullptr, L"Failed to initialize Direct3D 12.", L"Error", MB_OK);
        return false;
    }

    // アプリケーション固有の初期化
    if (!InitApp()) {
        TermD3D();
        MessageBoxW(
            nullptr, L"Failed to initialize application.", L"Error", MB_OK);
        return false;
    }

    // 描画領域の大きさに合わせてカメラのアスペクト比を設定
    ApplyRenderSize(width, height);

    return true;
}

// 終了処理
void Engine::Shutdown() {
    // GPUの処理が完了するまで待機
    m_Device.WaitForGPU();

    // アプリケーション固有の終了処理
    TermApp();

    // D3D終了処理
    TermD3D();
}

// フェンス待機・コマンドリスト/アロケータのリセット
void Engine::BeginFrame() {
    m_Scene.BeginFrame(m_Renderer.GetFrameIndex());

    m_Renderer.BeginFrame();

    m_DebugUI.BeginFrame(m_InputSystem, m_Scene.GetCamera(), m_Scene);
}

// 定数バッファの更新
void Engine::Update() {
    m_Renderer.UpdateConstants(m_Scene, m_DebugUI.GetDebugView());
}

// 描画コマンドの記録
void Engine::Render() {
    // シャドウマップの描画
    m_Renderer.BeginShadowPass();
    m_ShadowPass.Draw(m_Renderer.MakeShadowPassBindings(), m_Scene);
    m_Renderer.EndShadowPass();

    // シーンの描画
    m_Renderer.BeginScenePass();
    m_ScenePass.Draw(m_Renderer.MakeScenePassBindings(m_AssetSystem), m_Scene);

    // スカイボックスの描画
    m_SkyboxPass.Draw(m_Renderer.MakeSkyboxPassBindings(m_AssetSystem));

    // デバッグUIの描画
    m_DebugUI.Render(m_Renderer.GetUITarget(), m_Renderer.GetCommandList());

    // シーン描画とUI描画の合成
    m_Renderer.BeginCompositePass();
    m_CompositePass.Draw(m_Renderer.MakeCompositePassBindings());
}

// コマンドリスト実行，フェンス発行
// 描画コマンドの実行
void Engine::EndFrame() {
    // コマンドリストのクローズと実行，フェンス発行
    m_Renderer.EndFrame();
}

// フレーム表示
void Engine::Present() { m_Renderer.Present(); }

//==============================================
// private methods
//==============================================

// D3D12を動かすための初期化
bool Engine::InitD3D(HWND hWnd, uint32_t width, uint32_t height) {
    // デバイスとコマンドキュー，フェンス，ディスクリプタプールの生成
    if (!m_Device.Init()) {
        return false;
    }

    // ウィンドウハンドルの保存
    m_hWnd = hWnd;

    // Rendererの初期化
    if (!m_Renderer.Init(m_Device, width, height, hWnd)) {
        return false;
    }

    return true;
}

void Engine::TermD3D() {
    // GPUの処理が完了するまで待機
    m_Device.WaitForGPU();

    // Rendererの終了処理
    m_Renderer.Term();

    // コマンドキュー，デバイス，ディスクリプタプールの破棄
    m_Device.Term();
}

// アプリケーション固有の初期化
bool Engine::InitApp() {
    // シーンの初期化
    m_Scene.Init(m_Device);

    // アセット管理クラスの初期化
    if (!m_AssetSystem.Init(m_Device)) {
        return false;
    }

    // シャドウマップ描画パスの初期化
    if (!m_ShadowPass.Init(m_Device)) {
        OutputDebugStringW(L"Failed to initialize ShadowPass.\n");
        return false;
    }

    // シーン描画パスの初期化
    if (!m_ScenePass.Init(m_Device)) {
        OutputDebugStringW(L"Failed to initialize ScenePass.\n");
        return false;
    }

    // スカイボックス描画パスの初期化
    if (!m_SkyboxPass.Init(m_Device)) {
        OutputDebugStringW(L"Failed to initialize SkyboxPass.\n");
        return false;
    }

    // ImGuiの初期化
    if (!m_DebugUI.Init(m_Device, config::kUIBufferFormat, m_hWnd)) {
        MessageBoxW(nullptr, L"Failed to initialize ImGui.", L"Error", MB_OK);
        return false;
    }

    // UI合成パスの初期化
    if (!m_CompositePass.Init(m_Device)) {
        OutputDebugStringW(L"Failed to initialize CompositePass.\n");
        return false;
    }

    return true;
}

void Engine::TermApp() {
    // シーンの破棄
    m_Scene.Term();

    // アセット管理クラスの終了処理
    m_AssetSystem.Term();

    // デバッグUIの終了処理
    m_DebugUI.Term();

    // 描画パスの終了処理
    m_ScenePass.Term();
    m_SkyboxPass.Term();
    m_ShadowPass.Term();
    m_CompositePass.Term();
}

void Engine::ApplyRenderSize(uint32_t width, uint32_t height) {
    // 0除算の回避
    if (height == 0) {
        return;
    }

    // カメラのアスペクト比を更新
    m_Scene.GetCamera().SetAspect(
        static_cast<float>(width) / static_cast<float>(height));
}

// HDRIを読み込み，キューブマップを構築する
bool Engine::BuildEnvironmentMap(const std::filesystem::path& path) {
    return m_AssetSystem.BuildEnvironmentMap(path);
}

// AssetLoadScopeの作成
AssetLoadScope Engine::CreateAssetLoadScope() {
    return m_AssetSystem.CreateAssetLoadScope(m_Scene);
}

//=============================================
// イベント関数
//=============================================
void Engine::WindowEventAdapter::OnWindowMoved() {
    // モニター変更を検出
    if (m_pEngine->m_Renderer.DetectMonitorChange()) {
        m_pEngine->m_Renderer.QueryDisplayInfo();
        m_pEngine->m_Renderer.UploadDisplayConstants();
    }
}

void Engine::WindowEventAdapter::OnWindowResized(
    uint32_t width, uint32_t height) {
    // ウィンドウサイズ変更時の処理
    if (!m_pEngine->m_Renderer.ResizeBuffers(width, height)) {
        OutputDebugStringW(L"Failed to resize render targets.\n");
        return;
    }

    m_pEngine->ApplyRenderSize(width, height);
}
