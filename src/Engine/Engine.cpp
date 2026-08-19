//////////////////////////////////////////
/// @file Engine.cpp
/// @brief
//////////////////////////////////////////

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#include "Engine/Engine.h"

#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "Engine/Core/CommandQueue.h"
#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/DxDebug.h"
#include "Engine/Core/SwapChain.h"
#include "Engine/Debug/DebugUI.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/IndexBuffer.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Graphics/VertexBuffer.h"
#include "Engine/Model/MaterialGPU.h"
#include "Engine/Model/MeshGPU.h"
#include "Engine/Model/Model.h"
#include "Engine/Model/VertexTypes.h"
#include "Engine/Resource/AssetLoadScope.h"
#include "Engine/Resource/AssetPath.h"
#include "Engine/Resource/GLBImporter.h"
#include "Engine/Shader/SceneConstantsGPU.h"
#include "Engine/Shader/TransformGPU.h"

///////////////////////////////////////////
// Linker
///////////////////////////////////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
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
    SwapChain& sc = m_Renderer.GetSwapChain();

    // 1. DXGIフレームペーシング
    sc.WaitForFrameLatency();

    // 2. フェンス同期
    uint32_t frameIndex = sc.GetFrameIndex();
    uint64_t fenceValue = m_FrameResources[frameIndex].GetFenceValue();
    // 初回フレーム（fencevalue == 0）の場合は待機をスキップ
    if (fenceValue != 0) {
        m_Device.GetCommandQueue().Wait(fenceValue, INFINITE);
    }
    // 遅延解放キューのクリア
    m_Scene.BeginFrame(frameIndex);

    // 3. コマンドリスト/アロケータのリセット
    m_FrameResources[frameIndex].BeginFrame(m_pCmdList.Get());

    // 4. リソースバリア(Present -> RenderTarget)の設定
    ColorTarget& backBuffer        = sc.GetBackBuffer();
    DepthTarget& depthBuffer       = sc.GetDepthBuffer();
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = backBuffer.GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 5. レンダーターゲットとビューポートの設定・クリア
    // レンダーターゲットの設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = backBuffer.GetRTVCPUHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthBuffer.GetCPUHandle();
    BeginPass(m_pCmdList.Get(), kGeometryLayout, &rtvHandle, &dsvHandle);

    // レンダーターゲットのクリア
    const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    m_pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pCmdList->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定
    auto viewport    = sc.MakeViewport();
    auto scissorRect = sc.MakeScissorRect();
    m_pCmdList->RSSetViewports(1, &viewport);
    m_pCmdList->RSSetScissorRects(1, &scissorRect);

    m_DebugUI.BeginFrame(m_InputSystem, m_Scene.GetCamera(), m_Scene);
}

// ゲームロジック・シーン定数・transform更新
// GPUバッファへの書き込み
void Engine::Update() {
    uint32_t frameIndex = m_Renderer.GetSwapChain().GetFrameIndex();

    // 定数バッファの中身(行列やマテリアル情報)の更新
    // シーン内の全ゲームオブジェクトのtransformを更新
    m_Scene.ForEachObject(
        [&](GameObject& obj) { obj.UpdateTransformGPU(frameIndex); });

    // シーン内ライトの更新
    std::array<shader::LightConstants, config::kMaxLights> lights = {};
    uint32_t count = 0;  // 実際にコピーされたライトの数
    m_Scene.ForEachLight([&](Light& light) {
        if (!light.IsEnabled() || count >= config::kMaxLights) {
            return;
        }
        lights[count++] = light.ToShaderConstants();
    });
    uint32_t uploadedCount =  // バッファにコピーされたライトの数
        m_FrameResources[frameIndex].GetLightBuffer().Update(
            lights.data(), count);

    // シーン定数の更新
    shader::SceneConstants sc{};

    // ビュー行列・射影行列を転置して格納
    Camera& camera                 = m_Scene.GetCamera();
    DirectX::XMFLOAT4X4 view       = camera.GetViewMatrix();
    DirectX::XMFLOAT4X4 projection = camera.GetProjectionMatrix();
    DirectX::XMMATRIX viewMat      = DirectX::XMLoadFloat4x4(&view);
    DirectX::XMMATRIX projMat      = DirectX::XMLoadFloat4x4(&projection);
    DirectX::XMStoreFloat4x4(&sc.view, DirectX::XMMatrixTranspose(viewMat));
    DirectX::XMStoreFloat4x4(
        &sc.projection, DirectX::XMMatrixTranspose(projMat));

    // カメラ位置・時間・ライト数・露出・デバッグビューの設定
    sc.cameraPosition = camera.GetTransform().GetPosition();
    sc.time           = static_cast<float>(GetTickCount64()) / 1000.0f;
    sc.lightCount     = uploadedCount;  // 実際にアップロードされたライトの数
    sc.exposure       = camera.ComputeExposure();
    sc.debugView      = static_cast<uint32_t>(m_DebugUI.GetDebugView());

    m_FrameResources[frameIndex].GetSceneConstants().Update(sc);
}

// 描画コマンドの記録
void Engine::Render() {
    uint32_t frameIndex = m_Renderer.GetSwapChain().GetFrameIndex();

    // シーン描画処理
    {
        // パイプライン設定
        m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
        m_pCmdList->SetPipelineState(m_pPSO.Get());

        ID3D12DescriptorHeap* ppHeaps = { m_Device.CbvSrvUavPool()->GetHeap() };
        m_pCmdList->SetDescriptorHeaps(1, &ppHeaps);

        // [b0] SceneConstants (共通)
        m_pCmdList->SetGraphicsRootConstantBufferView(
            scene_rs::RootParam::CBV_Scene,
            m_FrameResources[frameIndex].GetSceneConstants().GetGPUAddress());

        // [b3] DisplayConstants (共通)
        m_pCmdList->SetGraphicsRootConstantBufferView(
            scene_rs::RootParam::CBV_Display,
            m_DisplayConstantsGPU.GetGPUAddress());

        // [t0, space1] IESプロファイルテクスチャ (共通)
        m_pCmdList->SetGraphicsRootDescriptorTable(
            scene_rs::RootParam::SRV_IESProfile,
            m_IESProfile.GetSrvGpuHandle());

        // [t0, space2] Light StructuredBuffer (共通)
        m_pCmdList->SetGraphicsRootDescriptorTable(
            scene_rs::RootParam::SRV_Lights,
            m_FrameResources[frameIndex].GetLightBuffer().GetGPUHandle());

        // PrimitiveTopologyの指定
        m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 全オブジェクトを描画
        m_Scene.ForEachObject([&](GameObject& obj) {
            // [b1] TransformConstants (モデル単位)
            m_pCmdList->SetGraphicsRootConstantBufferView(
                scene_rs::RootParam::CBV_Transform,
                obj.GetTransformGPU(frameIndex).GetGPUAddress());

            // 各メッシュを描画
            const auto model = m_Scene.GetModel(obj.GetModelHandle());
            if (model == nullptr) return;
            const auto& meshes    = model->GetMeshes();
            const auto& materials = model->GetMaterials();
            for (auto& mesh : meshes) {
                // このメッシュが使うマテリアルを取得
                uint32_t materialID = mesh->GetMaterialID();

                // マテリアルが存在しない場合は描画しない
                if (materialID >= materials.size() ||
                    materials[materialID] == nullptr) {
                    assert(false && "Mesh has no valid material.");
                    continue;
                }

                // マテリアルをバインド
                if (materialID < materials.size()) {
                    // [b2] MaterialConstants (マテリアル単位)
                    m_pCmdList->SetGraphicsRootConstantBufferView(
                        scene_rs::RootParam::CBV_Material,
                        materials[materialID]->GetConstantBufferGPUAddress());

                    // [t0-t4] PBR Textures
                    m_pCmdList->SetGraphicsRootDescriptorTable(
                        scene_rs::RootParam::SRV_Texture,
                        materials[materialID]->GetSrvTableBaseGPUHandle());
                }

                // 頂点バッファ・インデックスバッファの設定
                auto vbv = mesh->GetVertexBufferView();
                auto ibv = mesh->GetIndexBufferView();
                m_pCmdList->IASetVertexBuffers(0, 1, &vbv);
                m_pCmdList->IASetIndexBuffer(&ibv);

                // 描画コマンドの発行
                m_pCmdList->DrawIndexedInstanced(
                    mesh->GetIndexCount(), 1, 0, 0, 0);
            }
        });
    }

    // デバッグUIの描画
    m_DebugUI.Render(m_UIRenderTarget, m_pCmdList.Get());

    // シーン描画とUI描画の合成
    {
        // レンダーターゲットの設定
        auto rtvHandle =
            m_Renderer.GetSwapChain().GetBackBuffer().GetRTVCPUHandle();
        BeginPass(m_pCmdList.Get(), kCompositeLayout, &rtvHandle, nullptr);

        // ルートシグネチャとパイプラインステートの設定
        m_pCmdList->SetGraphicsRootSignature(m_pUIRootSignature.Get());
        m_pCmdList->SetPipelineState(m_pUIPSO.Get());

        // CBVとしてDisplayConstantsを設定
        m_pCmdList->SetGraphicsRootConstantBufferView(
            ui_rs::RootParam::CBV_Display,
            m_DisplayConstantsGPU.GetGPUAddress());

        // ディスクリプタヒープの設定
        ID3D12DescriptorHeap* pHeaps[] = {
            m_Device.CbvSrvUavPool()->GetHeap()
        };
        m_pCmdList->SetDescriptorHeaps(1, pHeaps);
        m_pCmdList->SetGraphicsRootDescriptorTable(
            ui_rs::RootParam::SRV_UI, m_UIRenderTarget.GetSRVGPUHandle());

        // ビューポートの設定
        auto viewport = m_Renderer.GetSwapChain().MakeViewport();
        m_pCmdList->RSSetViewports(1, &viewport);
        auto scissorRect = m_Renderer.GetSwapChain().MakeScissorRect();
        m_pCmdList->RSSetScissorRects(1, &scissorRect);

        // 描画
        m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pCmdList->DrawInstanced(3, 1, 0, 0);  // フルスクリーン三角形を描画
    }
}

// コマンドリスト実行，フェンス発行
// 描画コマンドの実行
void Engine::EndFrame() {
    uint32_t frameIndex = m_Renderer.GetSwapChain().GetFrameIndex();

    // 1. リソースバリアの設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource =
        m_Renderer.GetSwapChain().GetBackBuffer().GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 2. コマンドリストのクローズ
    m_pCmdList->Close();

    // 3. コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCmdList.Get() };
    m_Device.GetCommandQueue().Execute(
        ppCommandLists, _countof(ppCommandLists));

    // 4. フェンスの発行
    UINT64 fenceValue = m_Device.GetCommandQueue().Signal();

    // 5. フェンス値の保存
    m_FrameResources[frameIndex].EndFrame(fenceValue);
}

// 画面表示，フレームインデックス更新
// 結果の表示
void Engine::Present() {
    // 画面表示
    m_Renderer.GetSwapChain().Present();
}

//==============================================
// private methods
//==============================================

// D3D12を動かすための初期化
// デバイス，コマンドキュー，スワップチェインの生成
bool Engine::InitD3D(HWND hWnd, uint32_t width, uint32_t height) {
    // デバッグレイヤーの有効化
    dxdebug::EnableDebugLayer();

    // デバイスとコマンドキュー，フェンス，ディスクリプタプールの生成
    if (!m_Device.Init()) {
        return false;
    }

    // InfoQueueの設定
    dxdebug::SetupInfoQueue(m_Device.GetDevice());

    // ウィンドウハンドルの保存
    m_hWnd = hWnd;

    // Rendererの初期化
    if (!m_Renderer.GetSwapChain().Init(m_Device, width, height, hWnd)) {
        return false;
    }

    // UI用レンダーターゲットの作成
    {
        if (!m_UIRenderTarget.Init(m_Device.GetDevice(), m_Device.RtvPool(),
                m_Device.CbvSrvUavPool(), width, height,
                kUIRenderTargetFormat)) {
            return false;
        }
    }

    return true;
}

void Engine::TermD3D() {
    // GPUの処理が完了するまで待機
    m_Device.WaitForGPU();

    // UI用レンダーターゲットの解放
    m_UIRenderTarget.Term();

    // Rendererの終了処理
    m_Renderer.Term();

    // コマンドキュー，デバイス，ディスクリプタプールの破棄
    m_Device.Term();
}

// アプリケーション固有の初期化
// パイプライン，メッシュロード，バッファ生成など
bool Engine::InitApp() {
    // フレームリソースの初期化
    for (int i = 0; i < config::kFrameCount; i++) {
        if (!m_FrameResources[i].Init(m_Device)) {
            return false;
        }
    }

    // コマンドリストの生成
    {
        CHECK_HR(m_Device.GetDevice(),
            m_Device.GetDevice()->CreateCommandList(0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_FrameResources[m_Renderer.GetSwapChain().GetFrameIndex()]
                    .GetCommandAllocator(),
                nullptr, IID_PPV_ARGS(m_pCmdList.GetAddressOf())));
        m_pCmdList->Close();
    }

    // ファイルのロード
    {
        // ModelLoaderの初期化
        if (!m_modelLoader.Init(m_Device, &m_TextureManager)) {
            MessageBoxW(
                nullptr, L"Failed to initialize ModelLoader.", L"Error", MB_OK);
            return false;
        }

        // シーンの初期化
        m_Scene.Init(m_Device);

        // TextureManagerの初期化
        if (!m_TextureManager.Init(m_Device.GetDevice())) {
            MessageBoxW(nullptr, L"Failed to initialize TextureManager.",
                L"Error", MB_OK);
            return false;
        }

        // ResourceUploadBatchの生成
        DirectX::ResourceUploadBatch batch(m_Device.GetDevice());
        batch.Begin();

        // デフォルトテクスチャの生成
        if (!m_TextureManager.CreateDefaultTextures(batch)) {
            MessageBoxW(nullptr, L"Failed to create default textures.",
                L"Error", MB_OK);
            return false;
        }

        // IESProfileの初期化
        if (!m_IESProfile.Init(m_Device)) {
            MessageBoxW(
                nullptr, L"Failed to initialize IESProfile.", L"Error", MB_OK);
            return false;
        }

        // 転送完了を待機
        auto future = batch.End(m_Device.GetCommandQueue().GetD3DQueue());
        future.wait();

        // アップロードヒープの破棄
        m_Scene.DiscardModelUploads();
    }

    // ルートシグネチャの生成
    {
        RootSignatureBuilder builder;

        // SRVのレンジを作成
        // [t0-t4, space0] PBR Textures (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;
        range.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> iesRange;
        iesRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t0, space2] Light StructuredBuffer (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> lightRange;
        lightRange.push_back(RootSignatureBuilder::CreateRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2,
            D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE));

        // ルートシグニチャ構成
        // [b0] SceneConstants (Root CBV)
        // [b1] TransformConstants (Root CBV)
        // [b2] Material Constants (Root CBV)
        // [b3] Display Constants (Root CBV)
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        // baseColor, metallic-roughness, normal, emissive, occlusion
        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        // [t0, space2] Light StructuredBuffer (Descriptor Table SRV)
        // [s0] Default Sampler (Static Sampler)
        // [s1] IES Profile Sampler (Static Sampler)
        builder
            .SetFlags(
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
            .AddCBV(0, 0, D3D12_SHADER_VISIBILITY_ALL,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(1, 0, D3D12_SHADER_VISIBILITY_VERTEX,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddCBV(3, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(iesRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(lightRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddStaticSampler(0)
            .AddStaticSampler(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // 垂直角は端で止める
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // 水平角は0-360°でループする
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        bool result = builder.Build(m_Device.GetDevice());
        if (!result) {
            MessageBoxW(
                nullptr, L"Failed to build root signature.", L"Error", MB_OK);
            return false;
        }

        m_pRootSignature = builder.Get();
    }

    // パイプラインステートの生成
    {
        // シェーダの検索と読み込み
        std::filesystem::path vsPath;
        std::filesystem::path psPath;
        AssetPath assetPath;

        // シェーダのパスを取得
        if (!assetPath.GetAssetPath(L"shader/TestVS.cso", vsPath) ||
            !assetPath.GetAssetPath(L"shader/GGX_PS.cso", psPath)) {
            MessageBoxW(
                nullptr, L"Failed to find shader files.", L"Error", MB_OK);
            return false;
        }

        // シェーダの読み込み
        engine::ComPtr<ID3DBlob> vsBlob;
        engine::ComPtr<ID3DBlob> psBlob;
        CHECK_HR(m_Device.GetDevice(),
            D3DReadFileToBlob(vsPath.c_str(), vsBlob.GetAddressOf()));
        CHECK_HR(m_Device.GetDevice(),
            D3DReadFileToBlob(psPath.c_str(), psBlob.GetAddressOf()));

        // グラフィックスパイプラインステートの設定
        GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.SetRootSignature(m_pRootSignature.Get())
            .SetVertexShader(vsBlob.Get())
            .SetPixelShader(psBlob.Get())
            .SetInputLayout(StandardVertex::GetInputLayout())
            .SetBlendState(BlendMode::Opaque)
            .SetRenderTargetLayout(kGeometryLayout);

        if (!pipelineBuilder.Build(m_Device.GetDevice())) {
            MessageBoxW(nullptr, L"Failed to build graphics pipeline state.",
                L"Error", MB_OK);
            return false;
        }

        m_pPSO = pipelineBuilder.Get();
    }

    // ディスプレイ定数の初期化
    {
        m_DisplayInfo               = GetDisplayInfo();
        shader::DisplayConstants dc = {};
        dc.maxLuminance             = m_DisplayInfo.maxLuminance;
        dc.minLuminance             = m_DisplayInfo.minLuminance;
        dc.paperWhiteNits =
            m_DisplayInfo.isHDRSupported ? 200.0f : 80.0f;  // SDRの白
        dc.maxFullFrameLuminance = m_DisplayInfo.maxFullFrameLuminance;

        if (!m_DisplayConstantsGPU.Init(
                m_Device.GetDevice(), m_Device.CbvSrvUavPool(), dc)) {
            MessageBoxW(nullptr, L"Failed to initialize display constants.",
                L"Error", MB_OK);
            return false;
        }
    }

    // ImGuiの初期化
    if (!m_DebugUI.Init(m_Device, kUIRenderTargetFormat, m_hWnd)) {
        MessageBoxW(nullptr, L"Failed to initialize ImGui.", L"Error", MB_OK);
        return false;
    }

    // UI合成用PSO，ルートシグネチャの作成
    {
        // シェーダのパスを取得
        std::filesystem::path vsPath;
        std::filesystem::path psPath;
        AssetPath assetPath;
        if (!assetPath.GetAssetPath(L"shader/UI_VS.cso", vsPath) ||
            !assetPath.GetAssetPath(L"shader/UI_PS.cso", psPath)) {
            MessageBoxW(
                nullptr, L"Failed to find shader files.", L"Error", MB_OK);
            return false;
        }

        // シェーダの読み込み
        engine::ComPtr<ID3DBlob> vsBlob;
        engine::ComPtr<ID3DBlob> psBlob;
        CHECK_HR(m_Device.GetDevice(),
            D3DReadFileToBlob(vsPath.c_str(), vsBlob.GetAddressOf()));
        CHECK_HR(m_Device.GetDevice(),
            D3DReadFileToBlob(psPath.c_str(), psBlob.GetAddressOf()));

        // ルートシグネチャの生成
        // ルートシグネチャの構成
        // [b3] Display Constants (Root CBV)
        // [t0] UI Texture (Descriptor Table SRV)
        auto rsBuilder = RootSignatureBuilder{};
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;
        range.push_back(
            rsBuilder.CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
                D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE));
        rsBuilder
            .AddCBV(3, 0, D3D12_SHADER_VISIBILITY_ALL,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL);

        if (!rsBuilder.Build(m_Device.GetDevice())) {
            MessageBoxW(nullptr, L"Failed to build UI root signature.",
                L"Error", MB_OK);
            return false;
        }
        m_pUIRootSignature = rsBuilder.Get();

        // パイプラインステートの生成
        auto psoBuilder = GraphicsPipelineBuilder{};
        psoBuilder.SetRootSignature(m_pUIRootSignature.Get())
            .SetVertexShader(vsBlob.Get())
            .SetPixelShader(psBlob.Get())
            .SetBlendState(BlendMode::PremultipliedAlpha)
            .SetRenderTargetLayout(kCompositeLayout);

        if (!psoBuilder.Build(m_Device.GetDevice())) {
            MessageBoxW(nullptr, L"Failed to build UI pipeline state.",
                L"Error", MB_OK);
            return false;
        }
        m_pUIPSO = psoBuilder.Get();
    }

    return true;
}

void Engine::TermApp() {
    // ディスプレイ定数の破棄
    m_DisplayConstantsGPU.Term();

    // シーンの破棄
    m_Scene.Term();

    // テクスチャプールの解放
    m_TextureManager.Term();

    // デバッグUIの終了処理
    m_DebugUI.Term();

    // PSO，RootSignatureの破棄
    m_pPSO.Reset();
    m_pRootSignature.Reset();
    m_pUIPSO.Reset();
    m_pUIRootSignature.Reset();

    // フレームリソースの解放
    for (int i = 0; i < config::kFrameCount; i++) {
        m_FrameResources[i].Term();
    }

    // コマンドリストの解放
    m_pCmdList.Reset();
}

AssetLoadScope Engine::CreateAssetLoadScope() {
    // ResourceUploadBatchのBegin
    auto batch =
        std::make_unique<DirectX::ResourceUploadBatch>(m_Device.GetDevice());
    batch->Begin();

    // ModelLoadScopeの初期化
    return AssetLoadScope(std::move(batch), m_Device.GetCommandQueue(),
        m_modelLoader, m_Scene, m_IESProfile);
}

//=============================================
// 内部ヘルパー
//=============================================
/// @brief HDR対応チェック
DisplayInfo Engine::GetDisplayInfo() {
    // 出力情報の初期化
    DisplayInfo info           = {};
    info.isHDRSupported        = false;
    info.maxLuminance          = 80.0f;
    info.minLuminance          = 0.0f;
    info.maxFullFrameLuminance = 80.0f;

    // スワップチェーンから現座表示されているOutputを取得
    ComPtr<IDXGIOutput> output;
    if (FAILED(m_Renderer.GetSwapChain().GetSwapChain()->GetContainingOutput(
            output.GetAddressOf()))) {
        return info;
    };
    ComPtr<IDXGIOutput6> output6;
    if (FAILED(output.As(&output6))) {
        return info;
    }

    // ディスプレイの詳細情報を取得
    DXGI_OUTPUT_DESC1 desc1 = {};
    if (FAILED(output6->GetDesc1(&desc1))) {
        return info;
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

    return info;
}

bool Engine::IsMonitorChanged(HWND hWnd) {
    // 現在のモニターを取得
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);

    if (m_DisplayInfo.hMonitor != hMonitor) {
        return true;
    }
    return false;
}

//=============================================
// イベント関数
//=============================================
void Engine::WindowEventAdapter::OnWindowMoved() {
    // モニター変更チェック
    if (m_pEngine->IsMonitorChanged(m_pEngine->m_hWnd)) {
        // ディスプレイ情報取得
        DisplayInfo displayInfo = m_pEngine->GetDisplayInfo();

        // ディスプレイ定数の更新
        shader::DisplayConstants dc = {};
        dc.maxLuminance             = displayInfo.maxLuminance;
        dc.minLuminance             = displayInfo.minLuminance;
        dc.paperWhiteNits        = displayInfo.isHDRSupported ? 200.0f : 80.0f;
        dc.maxFullFrameLuminance = displayInfo.maxFullFrameLuminance;

        m_pEngine->m_DisplayInfo = displayInfo;
        m_pEngine->m_DisplayConstantsGPU.Update(dc);
    }

    // フレームレート設定
}

void Engine::WindowEventAdapter::OnDisplayChanged() {
    // ディスプレイ情報取得
    DisplayInfo displayInfo = m_pEngine->GetDisplayInfo();

    // ディスプレイ定数の更新
    shader::DisplayConstants dc = {};
    dc.maxLuminance             = displayInfo.maxLuminance;
    dc.minLuminance             = displayInfo.minLuminance;
    dc.paperWhiteNits           = displayInfo.isHDRSupported ? 200.0f : 80.0f;
    dc.maxFullFrameLuminance    = displayInfo.maxFullFrameLuminance;

    m_pEngine->m_DisplayInfo = displayInfo;
    m_pEngine->m_DisplayConstantsGPU.Update(dc);
}
