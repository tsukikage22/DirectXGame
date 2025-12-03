//////////////////////////////////////////
/// @file Engine.cpp
/// @brief
//////////////////////////////////////////

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#include "Engine/Engine.h"

////////////////////////////////////////////
// Engine class
////////////////////////////////////////////

// 初期化
bool Engine::Initialize() {
    // D3D初期化
    if (!InitD3D()) {
        return false;
    }

    // アプリケーション固有の初期化
    if (!InitApp()) {
        TermD3D();
        return false;
    }

    return true;
}

// 終了処理
void Engine::Shutdown() {
    // アプリケーション固有の終了処理
    TermApp();

    // D3D終了処理
    TermD3D();
}

// フェンス待機・コマンドリスト/アロケータのリセット
void Engine::BeginFrame() {
    // 1. フェンス同期
    uint64_t fenceValue = m_FrameResources[m_FrameIndex].GetFenceValue();
    m_CommandQueue.Wait(fenceValue, INFINITE);

    // 2. コマンドリスト/アロケータのリセット
    m_FrameResources[m_FrameIndex].BeginFrame(m_pCmdList.Get());

    // 3. リソースバリア(Present -> RenderTarget)の設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = m_ColorTarget[m_FrameIndex].GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 4. レンダーターゲットとビューポートの設定・クリア
    // レンダーターゲットの設定
    uint32_t RTVIndex = m_ColorTarget[m_FrameIndex].GetRTVIndex();
    uint32_t DSVIndex = m_pDepthTarget.GetDSVIndex();
    m_pCmdList->OMSetRenderTargets(1, &m_pPoolRTV->GetCPUHandle(RTVIndex),
        FALSE, &m_pPoolDSV->GetCPUHandle(DSVIndex));

    // レンダーターゲットのクリア
    const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    m_pCmdList->ClearRenderTargetView(
        m_pPoolRTV->GetCPUHandle(RTVIndex), clearColor, 0, nullptr);
    m_pCmdList->ClearDepthStencilView(m_pPoolDSV->GetCPUHandle(DSVIndex),
        D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定
    m_pCmdList->RSSetViewports(1, &m_Viewport);
    m_pCmdList->RSSetScissorRects(1, &m_ScissorRect);
}

// ゲームロジック・シーン定数・transform更新
// GPUバッファへの書き込み
void Engine::Update() {
    // 定数バッファの中身(行列やマテリアル情報)の更新
}

// 描画コマンドの記録
void Engine::Render() {
    // パイプライン設定
    m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    m_pCmdList->SetPipelineState(m_pPSO.Get());

    // 描画処理
    {
        ID3D12DescriptorHeap* ppHeaps = { m_pPoolCBV_SRV_UAV->GetHeap() };

        // [b0] SceneConstants
        m_pCmdList->SetGraphicsRootConstantBufferView(0,
            m_FrameResources[m_FrameIndex].GetSceneConstants().GetGPUAddress());

        // [b1] TransformConstants
        m_pCmdList->SetGraphicsRootConstantBufferView(1,
            m_FrameResources[m_FrameIndex].GetTransforms()[0].GetGPUAddress());

        // [b2] MaterialConstants
        m_pCmdList->SetGraphicsRootConstantBufferView(
            2, m_Materials[0].GetConstantBufferGPUAddress());

        // [t0-t4] PBR Textures
        m_pCmdList->SetDescriptorHeaps(1, &ppHeaps);

        m_pCmdList->DrawIndexedInstanced(
            m_Meshes[0].GetIndexCount(), 1, 0, 0, 0);
    }
}

// コマンドリスト実行，フェンス発行
// 描画コマンドの実行
void Engine::EndFrame() {
    // 1. リソースバリアの設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = m_ColorTarget[m_FrameIndex].GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 2. コマンドリストのクローズ
    m_pCmdList->Close();

    // 3. コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCmdList.Get() };
    m_CommandQueue.Execute(ppCommandLists, _countof(ppCommandLists));

    // 4. フェンスの発行
    UINT64 fenceValue = m_CommandQueue.Signal();

    // 5. フェンス値の保存
    m_FrameResources[m_FrameIndex].EndFrame(fenceValue);
}

// 画面表示，フレームインデックス更新
// 結果の表示
void Engine::Present() {
    // 画面表示
    m_pSwapChain->Present(1, 0);

    // フレームインデックス更新
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

//==============================================
// private methods
//==============================================

// D3D12を動かすための初期化
// デバイス，コマンドキュー，スワップチェインの生成
bool Engine::InitD3D() {
#if defined(_DEBUG)
    // デバッグレイヤーを有効化
    {
        engine::ComPtr<ID3D12Debug> debug;
        if (SUCCEEDED(
                D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())))) {
            debug->EnableDebugLayer();
        }
    }
#endif

    // デバイスの生成
    auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(m_pDevice.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    // コマンドキュー・フェンスの生成
    {
        if (!m_CommandQueue.Init(
                m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)) {
            return false;
        }
    }

    // スワップチェインの生成
    {
        // DXGIファクトリの生成
        engine::ComPtr<IDXGIFactory4> pFactory = nullptr;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));
        if (FAILED(hr)) {
            return false;
        }

        // スワップチェインの設定
        DXGI_SWAP_CHAIN_DESC desc               = {};
        desc.BufferDesc.Width                   = m_Width;
        desc.BufferDesc.Height                  = m_Height;
        desc.BufferDesc.RefreshRate.Numerator   = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count            = 1;
        desc.SampleDesc.Quality          = 0;
        desc.BufferUsage                 = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount                 = FrameCount;
        desc.OutputWindow                = m_hWnd;
        desc.Windowed                    = TRUE;
        desc.SwapEffect                  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // スワップチェインの生成
        engine::ComPtr<IDXGISwapChain> pSwapChain;
        hr = pFactory->CreateSwapChain(
            m_CommandQueue.GetD3DQueue(), &desc, pSwapChain.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        // IDXGISwapChain3を取得
        hr = pSwapChain.As(&m_pSwapChain);

        // バックバッファ番号を取得
        m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

        pFactory.Reset();
        pSwapChain.Reset();
    }

    // ディスクリプタプールの生成
    {
        // CBV/SRV/UAV
        if (!DescriptorPool::Create(m_pDevice.Get(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 2048,
                &m_pPoolCBV_SRV_UAV)) {
            return false;
        }

        // SMP
        if (!DescriptorPool::Create(m_pDevice.Get(),
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 256, &m_pPoolSMP)) {
            return false;
        }

        // RTV
        if (!DescriptorPool::Create(m_pDevice.Get(),
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                512, &m_pPoolRTV)) {
            return false;
        }

        // DSV
        if (!DescriptorPool::Create(m_pDevice.Get(),
                D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                512, &m_pPoolDSV)) {
            return false;
        }
    }

    // レンダーターゲットビューの生成
    {
        for (auto i = 0u; i < FrameCount; i++) {
            if (!m_ColorTarget[i].Init(
                    m_pDevice.Get(), m_pPoolRTV, i, m_pSwapChain.Get())) {
                return false;
            }
        }
    }

    // 深度ステンシルバッファの生成
    {
        if (!m_pDepthTarget.Init(m_pDevice.Get(), m_pPoolDSV, m_Width, m_Height,
                DXGI_FORMAT_D32_FLOAT)) {
            return false;
        }
    }

    // ビューポートの設定
    {
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.Width    = static_cast<float>(m_Width);
        m_Viewport.Height   = static_cast<float>(m_Height);
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
    }

    // シザー矩形の設定
    {
        m_ScissorRect.left   = 0;
        m_ScissorRect.top    = 0;
        m_ScissorRect.right  = m_Width;
        m_ScissorRect.bottom = m_Height;
    }

    return true;
}

void Engine::TermD3D() {
    // GPUの処理が完了するまで待機
    m_CommandQueue.Flush();

    // レンダーターゲットビューの解放
    for (auto i = 0u; i < FrameCount; i++) {
        m_ColorTarget[i].Term();
    }

    // 深度ステンシルビューの解放
    m_pDepthTarget.Term();

    // ディスクリプタプールの破棄
    delete m_pPoolCBV_SRV_UAV;
    delete m_pPoolSMP;
    delete m_pPoolRTV;
    delete m_pPoolDSV;

    // スワップチェインの破棄
    m_pSwapChain.Reset();

    // コマンドキュー・フェンス破棄
    m_CommandQueue.Term();

    // デバイスの破棄
    m_pDevice.Reset();
}

// アプリケーション固有の初期化
// パイプライン，メッシュロード，バッファ生成など
bool Engine::InitApp() {
    // フレームリソースの初期化
    for (int i = 0; i < FrameCount; i++) {
        m_FrameResources[i].Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV);
    }

    // コマンドリストの生成
    {
        auto hr =
            m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_FrameResources[m_FrameIndex].GetCommandAllocator(), nullptr,
                IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
        if (FAILED(hr)) {
            return false;
        }
        m_pCmdList->Close();
    }

    // 3Dモデルのロード
    {
        // ファイルの検索
        std::filesystem::path path;
        if (!AssetPath().GetAssetPath(L"box.fbx", path)) {
            return false;
        }

        // GLBの読み込み
        ModelAsset model;
        if (!GLBImporter::LoadFromFile(path, model)) {
            return false;
        }
        m_Models.push_back(model);
        m_textureCount = static_cast<UINT>(model.images.size());

        // オブジェクト数分のTransformを追加
        for (int i = 0; i < FrameCount; i++) {
            if (!m_FrameResources[i].AddTransform(
                    m_pDevice.Get(), m_pPoolCBV_SRV_UAV, m_Models.size())) {
                return false;
            }
        }

        // TextureManagerの初期化
        if (!m_TextureManager.Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV)) {
            return false;
        }

        // ResourceUploadBatchの生成
        DirectX::ResourceUploadBatch batch(m_pDevice.Get());
        batch.Begin();

        // デフォルトテクスチャの生成
        if (!m_TextureManager.CreateDefaultTextures(batch)) {
            return false;
        }

        // 全テクスチャの一括生成
        m_TextureManager.BuildTexturesFromModelAsset(model, batch);

        // メッシュをGPUに転送
        m_Meshes.resize(model.meshes.size());
        for (size_t i = 0; i < model.meshes.size(); i++) {
            if (!m_Meshes[i].Init(
                    m_pDevice.Get(), m_pCmdList.Get(), model.meshes[i])) {
                return false;
            }
        }

        // マテリアルをGPUに転送
        m_Materials.resize(model.materials.size());
        for (size_t i = 0; i < model.materials.size(); i++) {
            if (!m_Materials[i].Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV,
                    &m_TextureManager, model.materials[i])) {
                return false;
            }
        }

        // 転送完了を待機
        auto future = batch.End(m_CommandQueue.GetD3DQueue());
        future.wait();
    }

    // ルートシグニチャの生成
    {
        RootSignatureBuilder builder;

        // SRVのレンジを作成
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;
        range.push_back(RootSignatureBuilder::CreateRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0));

        // ルートシグニチャ構成
        // [b0] SceneConstants (Root CBV)
        // [b1] TransformConstants (Root CBV)
        // [b2] Material Constants (Root CBV)
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        // baseColor, metallic-roughness, normal, emissive, occlusion
        // [s0] Default Sampler (Static Sampler)
        builder
            .AddCBV(0, 0, D3D12_SHADER_VISIBILITY_ALL,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC)
            .AddCBV(1, 0, D3D12_SHADER_VISIBILITY_VERTEX,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddStaticSampler(0);

        bool result = builder.Build(m_pDevice.Get());
        if (!result) {
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
        if (!assetPath.GetAssetPath(L"BasicVS.cso", vsPath) ||
            !assetPath.GetAssetPath(L"BasicPS.cso", psPath)) {
            return false;
        }

        // シェーダの読み込み
        engine::ComPtr<ID3DBlob> vsBlob;
        engine::ComPtr<ID3DBlob> psBlob;
        auto hr = D3DReadFileToBlob(vsPath.c_str(), vsBlob.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }
        hr = D3DReadFileToBlob(psPath.c_str(), psBlob.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        // グラフィックスパイプラインステートの設定
        GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.SetDefault()
            .SetRootSignature(m_pRootSignature.Get())
            .SetVertexShader(vsBlob.Get())
            .SetPixelShader(psBlob.Get())
            .SetInputLayout(StandardVertex::GetInputLayout())
            .SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
            .SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

        if (!pipelineBuilder.Build(m_pDevice.Get())) {
            return false;
        }

        m_pPSO = pipelineBuilder.Get();
    }

    return true;
}

void Engine::TermApp() {
    // メッシュの解放
    for (auto& mesh : m_Meshes) {
        mesh.Term();
    }
    m_Meshes.clear();

    // マテリアルの解放
    for (auto& material : m_Materials) {
        material.Term();
    }
    m_Materials.clear();

    // テクスチャプールの解放
    m_TextureManager.Term();

    // パイプラインステートの解放
    m_pPSO.Reset();

    // ルートシグニチャの解放
    m_pRootSignature.Reset();

    // フレームリソースの解放
    for (int i = 0; i < FrameCount; i++) {
        m_FrameResources[i].Term();
    }

    // コマンドリストの解放
    m_pCmdList.Reset();
}