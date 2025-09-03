#include "Engine/ColorTarget.h"

bool ColorTarget::InitFromBackBuffer(ID3D12Device* pDevice,
    DescriptorPool* pPoolRTV, uint32_t index, IDXGISwapChain* pSwapChain) {
    // 引数チェック
    if (pDevice == nullptr || pPoolRTV == nullptr || pSwapChain == nullptr) {
        return false;
    }

    m_pPoolRTV = pPoolRTV;

    // バックバッファの情報を取得
    auto hr =
        pSwapChain->GetBuffer(index, IID_PPV_ARGS(m_pTarget.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    m_RTVIndex = m_pPoolRTV->Allocate();

    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);

    m_ViewDesc.Format = desc.BufferDesc.Format;
    m_ViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Texture2D.MipSlice = 0;
    m_ViewDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(
        m_pTarget.Get(), &m_ViewDesc, m_pPoolRTV->GetCPUHandle(m_RTVIndex));

    return true;
}

void ColorTarget::Term() {
    m_pTarget.Reset();

    // TODO: インデックスの値チェックも必要
    if (m_pPoolRTV != nullptr) {
        m_pPoolRTV->Free(m_RTVIndex);
    }
}

ColorTarget::ColorTarget()
    : m_pTarget(nullptr), m_pPoolRTV(nullptr), m_RTVIndex(0), m_ViewDesc{} {}

ColorTarget::~ColorTarget() { Term(); }