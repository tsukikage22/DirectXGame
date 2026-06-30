//////////////////////////////////////////
/// @file Engine.cpp
/// @brief
//////////////////////////////////////////

///////////////////////////////////////////
// Include
///////////////////////////////////////////
#include "Engine/Engine.h"

#include "Engine/Core/DxDebug.h"

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
    // アプリケーション固有の終了処理
    TermApp();

    // D3D終了処理
    TermD3D();
}

// フェンス待機・コマンドリスト/アロケータのリセット
void Engine::BeginFrame() {
    // 1. DXGIフレームペーシング
    WaitForSingleObjectEx(m_frameLatencyWaitableObject, 1000, TRUE);

    // 2. フェンス同期
    uint64_t fenceValue = m_FrameResources[m_FrameIndex].GetFenceValue();
    // 初回フレーム（fencevalue == 0）の場合は待機をスキップ
    if (fenceValue != 0) {
        m_CommandQueue.Wait(fenceValue, INFINITE);
    }

    // 3. コマンドリスト/アロケータのリセット
    m_FrameResources[m_FrameIndex].BeginFrame(m_pCmdList.Get());

    // 4. リソースバリア(Present -> RenderTarget)の設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = m_ColorTarget[m_FrameIndex].GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 5. レンダーターゲットとビューポートの設定・クリア
    // レンダーターゲットの設定
    uint32_t RTVIndex = m_ColorTarget[m_FrameIndex].GetRTVIndex();
    uint32_t DSVIndex = m_DepthTarget.GetDSVIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pPoolRTV->GetCPUHandle(RTVIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pPoolDSV->GetCPUHandle(DSVIndex);
    m_pCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

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
    // シーン定数の更新
    shader::SceneConstants sc = {};

    // シーン内の全ゲームオブジェクトのtransformを更新
    for (auto& obj : m_Scene.GetGameObjects()) {
        obj->UpdateTransformGPU(m_FrameIndex);
    }

    // ビュー行列・射影行列を転置して格納
    DirectX::XMFLOAT4X4 view       = m_Camera.GetViewMatrix();
    DirectX::XMFLOAT4X4 projection = m_Camera.GetProjectionMatrix();
    DirectX::XMMATRIX viewMat      = DirectX::XMLoadFloat4x4(&view);
    DirectX::XMMATRIX projMat      = DirectX::XMLoadFloat4x4(&projection);
    DirectX::XMStoreFloat4x4(&sc.view, DirectX::XMMatrixTranspose(viewMat));
    DirectX::XMStoreFloat4x4(
        &sc.projection, DirectX::XMMatrixTranspose(projMat));

    // カメラ位置・時間・露出の設定
    sc.cameraPosition = m_Camera.GetPosition();
    sc.time           = static_cast<float>(GetTickCount64()) / 1000.0f;
    sc.exposure       = 3.0f;

    m_FrameResources[m_FrameIndex].GetSceneConstants().Update(sc);
}

// 描画コマンドの記録
void Engine::Render() {
    // パイプライン設定
    m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    m_pCmdList->SetPipelineState(m_pPSO.Get());

    // 描画処理
    {
        ID3D12DescriptorHeap* ppHeaps = { m_pPoolCBV_SRV_UAV->GetHeap() };
        m_pCmdList->SetDescriptorHeaps(1, &ppHeaps);

        // [b0] SceneConstants (共通)
        m_pCmdList->SetGraphicsRootConstantBufferView(RootParam::CBV_Scene,
            m_FrameResources[m_FrameIndex].GetSceneConstants().GetGPUAddress());

        // [b3] LightingConstants (共通)
        m_pCmdList->SetGraphicsRootConstantBufferView(
            RootParam::CBV_Lighting, m_FrameResources[m_FrameIndex]
                                         .GetLightingConstants()
                                         .GetGPUAddress());

        // [b4] DisplayConstants (共通)
        m_pCmdList->SetGraphicsRootConstantBufferView(
            RootParam::CBV_Display, m_DisplayConstantsGPU.GetGPUAddress());

        // [t0, space1] IESプロファイルテクスチャ (共通)
        m_pCmdList->SetGraphicsRootDescriptorTable(
            RootParam::SRV_IESProfile, m_IESProfile.GetSrvGpuHandle());

        // PrimitiveTopologyの指定
        m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 全オブジェクトを描画
        for (auto& obj : m_Scene.GetGameObjects()) {
            // [b1] TransformConstants (モデル単位)
            m_pCmdList->SetGraphicsRootConstantBufferView(
                RootParam::CBV_Transform,
                obj->GetTransformGPU(m_FrameIndex).GetGPUAddress());

            // 各メッシュを描画
            const auto& meshes    = obj->GetModel().GetMeshes();
            const auto& materials = obj->GetModel().GetMaterials();
            for (auto& mesh : obj->GetModel().GetMeshes()) {
                // このメッシュが使うマテリアルを取得
                uint32_t materialID = mesh->GetMaterialID();

                // マテリアルをバインド
                if (materialID < materials.size()) {
                    // [b2] MaterialConstants (マテリアル単位)
                    m_pCmdList->SetGraphicsRootConstantBufferView(
                        RootParam::CBV_Material,
                        materials[materialID]->GetConstantBufferGPUAddress());

                    // [t0-t4] PBR Textures
                    m_pCmdList->SetGraphicsRootDescriptorTable(
                        RootParam::SRV_Texture,
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
        }
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
bool Engine::InitD3D(HWND hWnd, uint32_t width, uint32_t height) {
    // デバッグレイヤーの有効化
    dxdebug::EnableDebugLayer();

    // デバイスの生成
    {
        auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(m_pDevice.GetAddressOf()));
        if (FAILED(hr)) {
            OutputDebugStringW(L"Failed to create D3D12 Device.\n");
            return false;
        }
    }

    // InfoQueueの設定
    dxdebug::SetupInfoQueue(m_pDevice.Get());

    // ウィンドウハンドルの保存
    m_hWnd = hWnd;

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
        m_pFactory.Reset();
        CHECK_HR(
            m_pDevice.Get(), CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory)));

        // スワップチェインの設定
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width                 = width;
        desc.Height                = height;
        desc.Format                = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc.Stereo                = FALSE;
        desc.SampleDesc.Count      = 1;
        desc.SampleDesc.Quality    = 0;
        desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount           = FrameCount;
        desc.Scaling               = DXGI_SCALING_STRETCH;
        desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

        // スワップチェインの生成
        engine::ComPtr<IDXGISwapChain1> pSwapChain;
        CHECK_HR(m_pDevice.Get(),
            m_pFactory->CreateSwapChainForHwnd(m_CommandQueue.GetD3DQueue(),
                hWnd, &desc, nullptr, nullptr, pSwapChain.GetAddressOf()));

        // IDXGISwapChain3を取得
        CHECK_HR(m_pDevice.Get(), pSwapChain.As(&m_pSwapChain));

        // バックバッファ番号を取得
        m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

        // カラースペースの設定（scRGB対応）
        m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);

        // フレームレイテンシ待機オブジェクトの取得
        m_pSwapChain->SetMaximumFrameLatency(FrameCount);
        m_frameLatencyWaitableObject =
            m_pSwapChain->GetFrameLatencyWaitableObject();

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
        if (!m_DepthTarget.Init(m_pDevice.Get(), m_pPoolDSV, width, height,
                DXGI_FORMAT_D32_FLOAT)) {
            return false;
        }
    }

    // ビューポートの設定
    {
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.Width    = static_cast<float>(width);
        m_Viewport.Height   = static_cast<float>(height);
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
    }

    // シザー矩形の設定
    {
        m_ScissorRect.left   = 0;
        m_ScissorRect.top    = 0;
        m_ScissorRect.right  = width;
        m_ScissorRect.bottom = height;
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
    m_DepthTarget.Term();

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
        if (!m_FrameResources[i].Init(m_pDevice.Get(), m_pPoolCBV_SRV_UAV)) {
            return false;
        }
    }

    // コマンドリストの生成
    {
        CHECK_HR(m_pDevice.Get(),
            m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_FrameResources[m_FrameIndex].GetCommandAllocator(), nullptr,
                IID_PPV_ARGS(m_pCmdList.GetAddressOf())));
        m_pCmdList->Close();
    }

    // 3Dモデルのロード
    {
        // ファイルの検索
        std::filesystem::path earthPath, moonPath;
        if (!AssetPath().GetAssetPath(L"model/TextureSphere.glb", earthPath)) {
            OutputDebugStringW(L"Error: model not found.\n");
            MessageBoxW(nullptr, L"Failed to find TextureSphere file.",
                L"Error", MB_OK);
            return false;
        }
        if (!AssetPath().GetAssetPath(L"model/MoonSphere.glb", moonPath)) {
            OutputDebugStringW(L"Error: model not found.\n");
            MessageBoxW(
                nullptr, L"Failed to find MoonSphere file.", L"Error", MB_OK);
            return false;
        }

        // TextureManagerの初期化
        if (!m_TextureManager.Init(m_pDevice.Get())) {
            MessageBoxW(nullptr, L"Failed to initialize TextureManager.",
                L"Error", MB_OK);
            return false;
        }

        // ResourceUploadBatchの生成
        DirectX::ResourceUploadBatch batch(m_pDevice.Get());
        batch.Begin();

        // デフォルトテクスチャの生成
        if (!m_TextureManager.CreateDefaultTextures(batch)) {
            MessageBoxW(nullptr, L"Failed to create default textures.",
                L"Error", MB_OK);
            return false;
        }

        // IESプロファイルのロード
        // パスの取得
        std::filesystem::path iesPath;
        if (!AssetPath().GetAssetPath(L"ies/TopPost.IES", iesPath)) {
            OutputDebugStringW(L"Error: IES profile not found.\n");
            MessageBoxW(
                nullptr, L"Failed to find IES profile file.", L"Error", MB_OK);
            return false;
        }
        // IESProfileの初期化
        if (!m_IESProfile.Init(
                m_pDevice.Get(), m_pPoolCBV_SRV_UAV, iesPath, batch)) {
            MessageBoxW(
                nullptr, L"Failed to initialize IESProfile.", L"Error", MB_OK);
            return false;
        }

        // モデルのロード
        ModelLoader loader;
        if (!loader.Init(
                m_pDevice.Get(), m_pPoolCBV_SRV_UAV, &m_TextureManager)) {
            MessageBoxW(
                nullptr, L"Failed to initialize ModelLoader.", L"Error", MB_OK);
            return false;
        }
        auto earth = loader.LoadModel(earthPath, batch);
        if (!earth) {
            MessageBoxW(nullptr, L"Failed to load model.", L"Error", MB_OK);
            return false;
        }
        // auto moon  = loader.LoadModel(moonPath, batch);

        // モデルをシーンに追加
        auto pEarth = m_Scene.AddModel(std::move(earth));
        // auto pMoon  = m_Scene.AddModel(std::move(moon));

        // ゲームオブジェクトをシーンに追加
        uint32_t objectIndex  = AddGameObject(pEarth);
        uint32_t objectIndex2 = AddGameObject(pEarth);

        // 座標設定
        DirectX::XMFLOAT3 pos1 = { -1.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 pos2 = { 1.0f, 0.0f, 0.0f };
        m_Scene.GetGameObjects()[objectIndex]->GetTransform().SetPosition(pos1);
        m_Scene.GetGameObjects()[objectIndex2]->GetTransform().SetPosition(
            pos2);

        // 転送完了を待機
        auto future = batch.End(m_CommandQueue.GetD3DQueue());
        future.wait();

        // アップロードヒープの破棄
        m_Scene.DiscardModelUploads();
    }

    // ルートシグネチャの生成
    {
        RootSignatureBuilder builder;

        // SRVのレンジを作成
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;
        range.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t0, space1] IESプロファイルテクスチャ
        std::vector<D3D12_DESCRIPTOR_RANGE1> iesRange;
        iesRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // ルートシグニチャ構成
        // [b0] SceneConstants (Root CBV)
        // [b1] TransformConstants (Root CBV)
        // [b2] Material Constants (Root CBV)
        // [b3] Lighting Constants (Root CBV)
        // [b4] Display Constants (Root CBV)
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        // baseColor, metallic-roughness, normal, emissive, occlusion
        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        // [s0] Default Sampler (Static Sampler)
        // [s1] IES Profile Sampler (Static Sampler)
        builder
            .SetFlags(
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
            .AddCBV(RootParam::CBV_Scene, 0, D3D12_SHADER_VISIBILITY_ALL,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(RootParam::CBV_Transform, 0, D3D12_SHADER_VISIBILITY_VERTEX,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(RootParam::CBV_Material, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddCBV(RootParam::CBV_Lighting, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddCBV(RootParam::CBV_Display, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(iesRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddStaticSampler(0)
            .AddStaticSampler(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // 垂直角は端で止める
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // 水平角は0-360°でループする
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        bool result = builder.Build(m_pDevice.Get());
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
        CHECK_HR(m_pDevice.Get(),
            D3DReadFileToBlob(vsPath.c_str(), vsBlob.GetAddressOf()));
        CHECK_HR(m_pDevice.Get(),
            D3DReadFileToBlob(psPath.c_str(), psBlob.GetAddressOf()));

        // グラフィックスパイプラインステートの設定
        GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.SetDefault()
            .SetRootSignature(m_pRootSignature.Get())
            .SetVertexShader(vsBlob.Get())
            .SetPixelShader(psBlob.Get())
            .SetInputLayout(StandardVertex::GetInputLayout())
            .SetRTVFormat(DXGI_FORMAT_R16G16B16A16_FLOAT)
            .SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

        if (!pipelineBuilder.Build(m_pDevice.Get())) {
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
                m_pDevice.Get(), m_pPoolCBV_SRV_UAV, dc)) {
            MessageBoxW(nullptr, L"Failed to initialize display constants.",
                L"Error", MB_OK);
            return false;
        }
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

//=============================================
// 内部ヘルパー
//=============================================
/// @brief SceneへのGameObject追加とGPUリソースの割り当て
/// @return 追加したGameObjectのインデックス
uint32_t Engine::AddGameObject(Model* pModel) {
    // シーンにゲームオブジェクトを追加
    uint32_t objectIndex =
        m_Scene.CreateGameObject(pModel, m_pDevice.Get(), m_pPoolCBV_SRV_UAV);

    return objectIndex;
}

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
    if (FAILED(m_pSwapChain->GetContainingOutput(output.GetAddressOf()))) {
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
