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

#include "Engine/AssetPath.h"
#include "Engine/ColorTarget.h"
#include "Engine/ComPtr.h"
#include "Engine/CommandQueue.h"
#include "Engine/DepthStencil.h"
#include "Engine/DescriptorPool.h"
#include "Engine/GraphicsPipelineBuilder.h"
#include "Engine/IndexBuffer.h"
#include "Engine/RootSignatureBuilder.h"
#include "Engine/VertexBuffer.h"
#include "Engine/VertexTypes.h"

///////////////////////////////////////////
// Linker
///////////////////////////////////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

////////////////////////////////////////////
// Engine class
////////////////////////////////////////////
class Engine {
public:
    static constexpr uint32_t FrameCount = 2;  // フレームバッファ数

    HWND m_hWnd;        // ウィンドウハンドル
    uint32_t m_Width;   // ウィンドウ横幅
    uint32_t m_Height;  // ウィンドウ縦幅

    void Initialize();
    void Shutdown();
    void BeginFrame();
    void EndFrame();

protected:
    engine::ComPtr<ID3D12Device> m_pDevice;           // デバイス
    engine::ComPtr<IDXGISwapChain3> m_pSwapChain;     // スワップチェイン
    engine::ComPtr<ID3D12CommandList> m_CommandList;  // コマンドリスト
    engine::ComPtr<ID3D12CommandAllocator>
        m_pCmdAllocator[FrameCount];                       // コマンドアロケータ
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;  // コマンドリスト
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature;  // ルートシグネチャ
    engine::ComPtr<ID3D12PipelineState> m_pPSO;  // パイプラインステート

    uint32_t m_FrameIndex;  // 現在のフレーム番号

    DescriptorPool* m_pPoolCBV_SRV_UAV;  // CBV/SRV/UAV用ディスクリプタプール
    DescriptorPool* m_pPoolRTV;          // RTV用ディスクリプタプール
    DescriptorPool* m_pPoolDSV;          // DSV用ディスクリプタプール
    DescriptorPool* m_pPoolSMP;          // サンプラ用ディスクリプタプール

    ColorTarget m_ColorTarget[FrameCount];  // カラーターゲット
    DepthStencil m_pDepthTarget;            // 深度ステンシル
    CommandQueue m_CommandQueue;            // コマンドキュー
    D3D12_VIEWPORT m_Viewport;              // ビューポート
    D3D12_RECT m_ScissorRect;               // シザー矩形

private:
    /////////////////////////////////////////////////////////////////////////
    // private methods
    /////////////////////////////////////////////////////////////////////////
    bool InitD3D();
    void TermD3D();
    void InitApp();
    void TermApp();
};