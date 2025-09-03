#include "DepthStencil.h"

DepthStencil::DepthStencil()
    : m_pTarget(nullptr), m_pPoolDSV(nullptr), m_DSVIndex(0), m_ViewDesc{} {}

DepthStencil::~DepthStencil() { Term(); }

bool DepthStencil::Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV,
    uint32_t width, uint32_t height, DXGI_FORMAT format) {
    // 引数チェック
    if (!pDevice || !pPoolDSV || width == 0 || height == 0) {
        return false;
    }

    m_pPoolDSV = pPoolDSV;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = UINT64(width);
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // リソースの生成
    auto hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
        IID_PPV_ARGS(m_pTarget.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }
    m_DSVIndex = m_pPoolDSV->Allocate();

    m_ViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Format = format;
    m_ViewDesc.Texture2D.MipSlice = 0;
    m_ViewDesc.Flags = D3D12_DSV_FLAG_NONE;

    pDevice->CreateDepthStencilView(
        m_pTarget.Get(), &m_ViewDesc, m_pPoolDSV->GetCPUHandle(m_DSVIndex));

    return true;
}

void DepthStencil::Term() {
    m_pTarget.Reset();

    // TODO: インデックスの値チェックも必要
    if (m_pPoolDSV != nullptr) {
        m_pPoolDSV->Free(m_DSVIndex);
    }
}