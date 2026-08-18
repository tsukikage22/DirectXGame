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
#include <dxgi1_6.h>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/CommandQueue.h"
#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/FrameResource.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Debug/DebugUI.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/DepthTarget.h"
#include "Engine/Input/IWindowEventListener.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Resource/IESProfile.h"
#include "Engine/Resource/ModelLoader.h"
#include "Engine/Resource/TextureManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Shader/DisplayConstantsGPU.h"

namespace scene_rs {
// ルートシグネチャ内でのルートパラメータ番号
// Addxxxの呼び出し順と一致させる
enum RootParam {
    CBV_Scene      = 0,  // b0
    CBV_Transform  = 1,  // b1
    CBV_Material   = 2,  // b2
    CBV_Display    = 3,  // b3
    SRV_Texture    = 4,  // t0-t4
    SRV_IESProfile = 5,  // t0, space1
    SRV_Lights     = 6,  // t0, space2
};

}  // namespace scene_rs

namespace ui_rs {
enum RootParam {
    CBV_Display = 0,  // b3
    SRV_UI      = 1,  // t0
};
}  // namespace ui_rs

/// @brief ディスプレイ情報
struct DisplayInfo {
    HMONITOR hMonitor;
    bool isHDRSupported;
    float maxLuminance;
    float minLuminance;
    float maxFullFrameLuminance;
};

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

    /// @brief モデルロード用オブジェクトの作成
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
    // constants
    //==============================================================
    static constexpr DXGI_FORMAT kBackBufferFormat =
        DXGI_FORMAT_R16G16B16A16_FLOAT;
    static constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_D32_FLOAT;
    static constexpr DXGI_FORMAT kUIRenderTargetFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    //==============================================================
    // Inner Class
    //==============================================================
    /// @brief ウィンドウイベント用の内部クラス
    class WindowEventAdapter : public IWindowEventListener {
    public:
        explicit WindowEventAdapter(Engine* pEngine) : m_pEngine(pEngine) {}

        /// @brief ウィンドウ移動時の処理
        void OnWindowMoved() override;

        void OnDisplayChanged() override;

    private:
        Engine* m_pEngine;
    };

    //==============================================================
    // private variables
    //==============================================================
    engine::ComPtr<IDXGISwapChain3> m_pSwapChain;          // スワップチェイン
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;  // パイプラインステート
    engine::ComPtr<IDXGIFactory6> m_pFactory;    // DXGIファクトリ

    uint32_t m_FrameIndex;  // 現在のフレーム番号

    DescriptorPool* m_pPoolCBV_SRV_UAV;  // CBV/SRV/UAV用ディスクリプタプール
    DescriptorPool* m_pPoolRTV;          // RTV用ディスクリプタプール
    DescriptorPool* m_pPoolDSV;          // DSV用ディスクリプタプール
    DescriptorPool* m_pPoolSMP;          // サンプラ用ディスクリプタプール

    GraphicsDevice m_Device;  // D3D12デバイスの管理クラス
    ColorTarget m_ColorTarget[config::kFrameCount];  // カラーターゲット
    DepthTarget m_DepthTarget;                       // 深度ステンシル
    CommandQueue m_CommandQueue;                     // コマンドキュー
    D3D12_VIEWPORT m_Viewport;                       // ビューポート
    D3D12_RECT m_ScissorRect;                        // シザー矩形

    FrameResource m_FrameResources[config::kFrameCount];  // フレームリソース

    ModelLoader m_modelLoader;
    TextureManager m_TextureManager;  // テクスチャマネージャ
    Scene m_Scene;                    // シーン

    IESProfile m_IESProfile;  // IESプロファイル

    InputSystem m_InputSystem;                  // 入力システム
    DisplayInfo m_DisplayInfo;                  // ディスプレイ情報
    DisplayConstantsGPU m_DisplayConstantsGPU;  // ディスプレイ定数GPU

    HANDLE m_frameLatencyWaitableObject =
        nullptr;  // フレームレイテンシ待機オブジェクト

    HWND m_hWnd;  // ウィンドウハンドル

    WindowEventAdapter m_WindowEventAdapter{ this };

    ColorTarget m_UIRenderTarget;  // UI用レンダーターゲット
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

    //==============================================================
    // 内部ヘルパー
    //==============================================================
    /// @brief HDR対応チェック
    DisplayInfo GetDisplayInfo();

    /// @brief モニター変更チェック
    bool IsMonitorChanged(HWND hWnd);
};