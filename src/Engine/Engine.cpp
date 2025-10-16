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

    // コマンドアロケータの生成
    {
        for (auto i = 0u; i < FrameCount; i++) {
            hr = m_pDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));
            if (FAILED(hr)) {
                return false;
            }
        }
    }

    // コマンドリストの生成
    {
        hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_pCmdAllocator[m_FrameIndex].Get(), nullptr,
            IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
        if (FAILED(hr)) {
            return false;
        }
    }

    // レンダーターゲットビューの生成
    {
        for (auto i = 0u; i < FrameCount; i++) {
            if (!m_ColorTarget[i].InitFromBackBuffer(
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

    // コマンドリストの解放
    m_pCmdList.Reset();

    // コマンドアロケータの解放
    for (auto i = 0u; i < FrameCount; i++) {
        m_pCmdAllocator[i].Reset();
    }

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
void Engine::InitApp() {
    // メッシュのロード
    {
        // ファイルの検索
        std::filesystem::path path;
        if (!AssetPath().GetAssetPath(L"box.fbx", path)) {
            return;
        }

        // GLBの読み込み
        ModelAsset model;
        if (!GLBImporter::LoadFromFile(path, model)) {
            return;
        }

        // TexturePoolの初期化
        if (!m_TexturePool.Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV)) {
            return;
        }

        // ResourceUploadBatchの生成
        DirectX::ResourceUploadBatch batch(m_pDevice.Get());
        batch.Begin();

        // デフォルトテクスチャの生成
        m_TexturePool.CreateDefaultTexture(batch);

        // 全テクスチャの一括生成
        m_TexturePool.CreateFromImages(model.images, batch);

        // メッシュをGPUに転送
        m_Meshes.resize(model.meshes.size());
        for (size_t i = 0; i < model.meshes.size(); i++) {
            if (!m_Meshes[i].Init(
                    m_pDevice.Get(), m_pCmdList.Get(), model.meshes[i])) {
                return;
            }
        }

        // マテリアルをGPUに転送
        m_Materials.resize(model.materials.size());
        for (size_t i = 0; i < model.materials.size(); i++) {
            if (!m_Materials[i].Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV,
                    &m_TexturePool, batch, model.materials[i])) {
                return;
            }
        }

        // 転送完了を待機
        auto future = batch.End(m_CommandQueue.GetD3DQueue());
        future.wait();
    }

    // ルートシグニチャの生成
    {
        RootSignatureBuilder builder;
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;

        range.push_back(RootSignatureBuilder::CreateRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0));

        bool result =
            builder.AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX)
                .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
                .AddStaticSampler(0)
                .Build(m_pDevice.Get());

        if (!result) {
            return;
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
            return;
        }

        // シェーダの読み込み
        engine::ComPtr<ID3DBlob> vsBlob;
        engine::ComPtr<ID3DBlob> psBlob;
        auto hr = D3DReadFileToBlob(vsPath.c_str(), vsBlob.GetAddressOf());
        if (FAILED(hr)) {
            return;
        }
        hr = D3DReadFileToBlob(psPath.c_str(), psBlob.GetAddressOf());
        if (FAILED(hr)) {
            return;
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
            return;
        }

        m_pPSO = pipelineBuilder.Get();
    }

    // ビューポートとシザー矩形
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
    m_TexturePool.Term();

    // パイプラインステートの解放
    m_pPSO.Reset();

    // ルートシグネチャの解放
    m_pRootSignature.Reset();
}

void Engine::BeginFrame() {}

void Engine::EndFrame() {}

/*
void Engine::Present() {
    // 画面表示
    m_pSwapChain->Present(1, 0);

    // 完了待ち
    m_CommandQueue.Flush();

    // フレーム番号を更新
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}
*/