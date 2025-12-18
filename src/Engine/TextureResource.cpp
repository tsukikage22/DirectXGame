#include "Engine/TextureResource.h"

#include "Engine/DxDebug.h"

TextureResource::TextureResource()
    : m_pResource(nullptr), m_width(0), m_height(0), m_mipLevels(0) {}

TextureResource::~TextureResource() { Term(); }

bool TextureResource::InitFromSwapChain(
    IDXGISwapChain* pSwapChain, UINT bufferIndex) {
    // 引数チェック
    if (pSwapChain == nullptr) {
        return false;
    }

    // 初期化
    Term();

    // スワップチェーンからバッファを取得
    auto hr = pSwapChain->GetBuffer(
        bufferIndex, IID_PPV_ARGS(m_pResource.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool TextureResource::InitAsTexture2D(ID3D12Device* pDevice, UINT width,
    UINT height, DXGI_FORMAT format, UINT mipLevels, D3D12_RESOURCE_FLAGS flags,
    D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* pClearValue) {
    // 引数チェック
    if (pDevice == nullptr || width == 0 || height == 0) {
        return false;
    }

    // ヒーププロパティの設定
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                  = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask      = 1;
    prop.VisibleNodeMask       = 1;

    // リソースディスクリプタの設定
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment           = 0;
    desc.Width               = static_cast<UINT64>(width);
    desc.Height              = height;
    desc.DepthOrArraySize    = 1;
    desc.MipLevels           = mipLevels;
    desc.Format              = format;
    desc.SampleDesc.Count    = 1;
    desc.SampleDesc.Quality  = 0;
    desc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags               = flags;

    // リソースの生成
    CHECK_HR(pDevice,
        pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
            initState, pClearValue, IID_PPV_ARGS(m_pResource.GetAddressOf())));

    m_width     = width;
    m_height    = height;
    m_mipLevels = mipLevels;

    return true;
}

void TextureResource::Term() { m_pResource.Reset(); }