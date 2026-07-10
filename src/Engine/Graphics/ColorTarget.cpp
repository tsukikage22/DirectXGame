#include "Engine/Graphics/ColorTarget.h"

ColorTarget::ColorTarget() : m_Target(), m_pPoolRTV(nullptr), m_ViewDesc{} {}

ColorTarget::~ColorTarget() { Term(); }

bool ColorTarget::Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV,
    uint32_t index, IDXGISwapChain* pSwapChain) {
    // 引数チェック
    if (pDevice == nullptr || pPoolRTV == nullptr || pSwapChain == nullptr) {
        return false;
    }

    // スワップチェーンからリソースを初期化
    if (!m_Target.InitFromSwapChain(pSwapChain, index)) {
        return false;
    }

    // RTVの作成
    m_pPoolRTV      = pPoolRTV;
    m_RTVAllocation = m_pPoolRTV->Allocate();

    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);

    m_ViewDesc.Format               = desc.BufferDesc.Format;
    m_ViewDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Texture2D.MipSlice   = 0;
    m_ViewDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(
        m_Target.GetResource(), &m_ViewDesc, m_RTVAllocation.GetCPUHandle());

    return true;
}

void ColorTarget::Term() {
    m_Target.Term();
    m_pPoolRTV = nullptr;
}
