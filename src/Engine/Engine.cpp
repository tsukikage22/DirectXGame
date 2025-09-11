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

    // 頂点バッファの生成
    {
    }

    // インデックスバッファの生成
    {
        size_t size;
        DXGI_FORMAT format;

        IndexBuffer indexBuffer(size, format);
        if (!indexBuffer.CreateIB(m_pDevice.Get(), m_pCmdList.Get(), nullptr)) {
            return;
        }
    }

    // 定数バッファの生成
    {
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
                .Build(m_pDevice.Get(), m_pRootSignature.GetAddressOf());

        if (result) {
            return;
        }
    }

    // パイプラインステートの生成
    {
        // シェーダの読み込み
    }

    // テクスチャの生成
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