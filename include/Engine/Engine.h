////////////////////////////////////////
/// @file Engine.h
/// @brief
////////////////////////////////////////

#pragma once

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>

#include <memory>
#include <vector>

#include "Engine/AssetPath.h"
#include "Engine/Camera.h"
#include "Engine/ColorTarget.h"
#include "Engine/ComPtr.h"
#include "Engine/CommandQueue.h"
#include "Engine/DepthTarget.h"
#include "Engine/DescriptorPool.h"
#include "Engine/FrameResource.h"
#include "Engine/GLBImporter.h"
#include "Engine/GraphicsPipelineBuilder.h"
#include "Engine/IndexBuffer.h"
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

protected:
    engine::ComPtr<ID3D12Device> m_pDevice;                // デバイス
    engine::ComPtr<IDXGISwapChain3> m_pSwapChain;          // スワップチェイン
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;  // パイプラインステート

    uint32_t m_FrameIndex;       // 現在のフレーム番号
    size_t m_maxObjects = 1000;  // 最大オブジェクト数

    DescriptorPool* m_pPoolCBV_SRV_UAV;  // CBV/SRV/UAV用ディスクリプタプール
    DescriptorPool* m_pPoolRTV;          // RTV用ディスクリプタプール
    DescriptorPool* m_pPoolDSV;          // DSV用ディスクリプタプール
    DescriptorPool* m_pPoolSMP;          // サンプラ用ディスクリプタプール

    ColorTarget m_ColorTarget[FrameCount];  // カラーターゲット
    DepthTarget m_pDepthTarget;             // 深度ステンシル
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

private:
    /////////////////////////////////////////////////////////////////////////
    // private methods
    /////////////////////////////////////////////////////////////////////////
    bool InitD3D(HWND hWnd, uint32_t width, uint32_t height);
    void TermD3D();
    bool InitApp();
    void TermApp();
};