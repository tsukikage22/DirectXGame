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
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "Engine/AssetPath.h"
#include "Engine/Camera.h"
#include "Engine/ColorTarget.h"
#include "Engine/ComPtr.h"
#include "Engine/CommandQueue.h"
#include "Engine/DepthTarget.h"
#include "Engine/DescriptorPool.h"
#include "Engine/DisplayConstantsGPU.h"
#include "Engine/FrameResource.h"
#include "Engine/GLBImporter.h"
#include "Engine/GraphicsPipelineBuilder.h"
#include "Engine/IWindowEventListener.h"
#include "Engine/IndexBuffer.h"
#include "Engine/InputSystem.h"
#include "Engine/MaterialGPU.h"
#include "Engine/MeshGPU.h"
#include "Engine/RootSignatureBuilder.h"
#include "Engine/SceneConstantsGPU.h"
#include "Engine/TextureManager.h"
#include "Engine/TransformGPU.h"
#include "Engine/VertexBuffer.h"
#include "Engine/VertexTypes.h"

///////////////////////////////////////////
// Linker
///////////////////////////////////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "DirectXTex.lib")

enum RootParam {
    CBV_Scene     = 0,
    CBV_Transform = 1,
    CBV_Material  = 2,
    SRV_Texture   = 3
};

struct DisplayInfo {
    HMONITOR hMonitor;
    bool isHDRSupported;
    float maxLuminance;
    float minLuminance;
    float maxFullFrameLuminance;
};

////////////////////////////////////////////
// Engine class
////////////////////////////////////////////
class Engine {
public:
    static constexpr uint32_t FrameCount = 2;  // フレームバッファ数

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

    //==================================================================
    // アクセサ
    //==================================================================
    InputSystem& GetInputSystem() { return m_InputSystem; }

    IWindowEventListener& GetWindowEventListener() {
        return m_WindowEventAdapter;
    }

    Camera& GetCamera() { return m_Camera; }

private:
    //==============================================================
    // private variables
    //==============================================================

    engine::ComPtr<ID3D12Device> m_pDevice;                // デバイス
    engine::ComPtr<IDXGISwapChain3> m_pSwapChain;          // スワップチェイン
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;  // パイプラインステート
    engine::ComPtr<IDXGIFactory6> m_pFactory;    // DXGIファクトリ

    uint32_t m_FrameIndex;       // 現在のフレーム番号
    size_t m_maxObjects = 1000;  // 最大オブジェクト数

    DescriptorPool* m_pPoolCBV_SRV_UAV;  // CBV/SRV/UAV用ディスクリプタプール
    DescriptorPool* m_pPoolRTV;          // RTV用ディスクリプタプール
    DescriptorPool* m_pPoolDSV;          // DSV用ディスクリプタプール
    DescriptorPool* m_pPoolSMP;          // サンプラ用ディスクリプタプール

    ColorTarget m_ColorTarget[FrameCount];  // カラーターゲット
    DepthTarget m_DepthTarget;              // 深度ステンシル
    CommandQueue m_CommandQueue;            // コマンドキュー
    D3D12_VIEWPORT m_Viewport;              // ビューポート
    D3D12_RECT m_ScissorRect;               // シザー矩形

    FrameResource m_FrameResources[FrameCount];  // フレームリソース

    std::vector<ModelAsset> m_Models;                       // モデルデータ
    std::vector<std::unique_ptr<MeshGPU>> m_Meshes;         // メッシュデータ
    std::vector<std::unique_ptr<MaterialGPU>> m_Materials;  // マテリアルデータ
    UINT m_textureCount = 0;                                // テクスチャ数
    TextureManager m_TextureManager;  // テクスチャマネージャ
    Camera m_Camera;                  // カメラ

    static constexpr size_t maxObjects = 100;  // 最大オブジェクト数

    InputSystem m_InputSystem;                  // 入力システム
    DisplayInfo m_DisplayInfo;                  // ディスプレイ情報
    DisplayConstantsGPU m_DisplayConstantsGPU;  // ディスプレイ定数GPU

    HWND m_hWnd;  // ウィンドウハンドル

private:
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

    bool IsMonitorChanged(HWND hWnd);

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

    WindowEventAdapter m_WindowEventAdapter{ this };
};