////////////////////////////////////////
/// @file Engine.h
/// @brief
////////////////////////////////////////

#pragma once

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#define NOMINMAX
#include <Windows.h>
#include <d3d12.h>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Core/Renderer.h"
#include "Engine/Debug/DebugUI.h"
#include "Engine/Input/IWindowEventListener.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Render/ScenePass.h"
#include "Engine/Resource/AssetSystem.h"
#include "Engine/Scene/Scene.h"

namespace ui_rs {
enum RootParam {
    CBV_Display = 0,  // b3
    SRV_UI      = 1,  // t0
};
}  // namespace ui_rs

// 前方宣言
class AssetLoadScope;

////////////////////////////////////////////
// Engine class
////////////////////////////////////////////
class Engine {
public:
    //==================================================================
    // ライフサイクル管理
    //==================================================================
    bool Initialize(HWND hWnd, uint32_t width, uint32_t height);

    void Shutdown();

    //==================================================================
    // フレーム制御
    //==================================================================
    void BeginFrame();

    void Update();

    void Render();

    void EndFrame();

    void Present();

    /// @brief アセットロード用オブジェクトの作成
    AssetLoadScope CreateAssetLoadScope();

    //==================================================================
    // アクセサ
    //==================================================================
    InputSystem& GetInputSystem() { return m_InputSystem; }

    IWindowEventListener& GetWindowEventListener() {
        return m_WindowEventAdapter;
    }

    Scene& GetScene() { return m_Scene; }

private:
    //==============================================================
    // Inner Class
    //==============================================================
    /// @brief ウィンドウイベント用の内部クラス
    class WindowEventAdapter : public IWindowEventListener {
    public:
        explicit WindowEventAdapter(Engine* pEngine) : m_pEngine(pEngine) {}

        /// @brief ウィンドウ移動時の処理
        void OnWindowMoved() override;

        /// @brief ウィンドウサイズ変更時の処理
        void OnWindowResized(uint32_t width, uint32_t height) override;

    private:
        Engine* m_pEngine;
    };

    //==============================================================
    // private variables
    //==============================================================
    GraphicsDevice m_Device;  // D3D12デバイスの管理クラス
    Renderer m_Renderer;      // レンダラーの管理クラス
    ScenePass m_ScenePass;    // シーン描画パスの管理クラス

    AssetSystem m_AssetSystem;  // モデル読み込みなどのアセット管理クラス
    Scene m_Scene;              // シーン

    InputSystem m_InputSystem;  // 入力システム

    HWND m_hWnd;  // ウィンドウハンドル

    WindowEventAdapter m_WindowEventAdapter{ this };

    engine::ComPtr<ID3D12RootSignature>
        m_pUIRootSignature;                        // UI用ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pUIPSO;  // UI用パイプラインステート
    DebugUI m_DebugUI;                             // デバッグUI

    /////////////////////////////////////////////////////////////////////////
    // private methods
    /////////////////////////////////////////////////////////////////////////
    bool InitD3D(HWND hWnd, uint32_t width, uint32_t height);
    void TermD3D();
    bool InitApp();
    void TermApp();

    /// @brief 描画領域の大きさに合わせてカメラのアスペクト比を更新する
    void ApplyRenderSize(uint32_t width, uint32_t height);

    /// @brief シェーダーを読み込む
    /// @param filename シェーダーのファイル名
    /// @param outBlob 出力先
    [[nodiscard]] bool LoadShader(
        const wchar_t* filename, engine::ComPtr<ID3DBlob>& outBlob);
};
