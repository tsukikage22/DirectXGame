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

#include <filesystem>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Debug/DebugUI.h"
#include "Engine/Input/IWindowEventListener.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Render/CompositePass.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/ScenePass.h"
#include "Engine/Resource/AssetSystem.h"
#include "Engine/Scene/Scene.h"

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
    /// @brief 初期化
    /// @param hWnd ウィンドウハンドル
    /// @param width 描画領域の幅
    /// @param height 描画領域の高さ
    bool Initialize(HWND hWnd, uint32_t width, uint32_t height);

    /// @brief 終了処理
    void Shutdown();

    //==================================================================
    // フレーム制御
    //==================================================================
    /// @brief フレーム開始時の処理
    void BeginFrame();

    /// @brief 定数バッファの更新
    void Update();

    /// @brief 描画コマンドの記録
    void Render();

    /// @brief フレーム終了時の処理
    void EndFrame();

    /// @brief フレーム表示
    void Present();

    /// @brief HDRIを読み込み，キューブマップを構築する
    /// @param path HDRIファイルのパス
    /// @return 読み込みに成功したかどうか
    [[nodiscard]] bool BuildEnvironmentMap(const std::filesystem::path& path);

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
    GraphicsDevice m_Device;        // D3D12デバイスの管理クラス
    Renderer m_Renderer;            // レンダラーの管理クラス
    ScenePass m_ScenePass;          // シーン描画パスの管理クラス
    CompositePass m_CompositePass;  // UI合成パスの管理クラス

    AssetSystem m_AssetSystem;  // モデル読み込みなどのアセット管理クラス
    Scene m_Scene;              // シーン

    InputSystem m_InputSystem;  // 入力システム

    HWND m_hWnd;  // ウィンドウハンドル

    WindowEventAdapter m_WindowEventAdapter{ this };

    DebugUI m_DebugUI;  // デバッグUI

    /////////////////////////////////////////////////////////////////////////
    // private methods
    /////////////////////////////////////////////////////////////////////////
    bool InitD3D(HWND hWnd, uint32_t width, uint32_t height);
    void TermD3D();
    bool InitApp();
    void TermApp();

    /// @brief 描画領域の大きさに合わせてカメラのアスペクト比を更新する
    void ApplyRenderSize(uint32_t width, uint32_t height);
};
