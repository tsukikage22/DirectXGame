#include "Engine/ColorTarget.h"

ColorTarget::ColorTarget()
    : m_Target(), m_pPoolRTV(nullptr), m_RTVIndex(UINT32_MAX), m_ViewDesc{} {}

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
    m_pPoolRTV = pPoolRTV;
    m_RTVIndex = m_pPoolRTV->Allocate();

    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);

    m_ViewDesc.Format               = DXGI_FORMAT_R16G16B16A16_FLOAT;
    m_ViewDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Texture2D.MipSlice   = 0;
    m_ViewDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(m_Target.GetResource(), &m_ViewDesc,
        m_pPoolRTV->GetCPUHandle(m_RTVIndex));

    return true;
}

void ColorTarget::Term() {
    m_Target.Term();

    if (m_pPoolRTV != nullptr && m_RTVIndex != UINT32_MAX) {
        m_pPoolRTV->Free(m_RTVIndex);
        m_RTVIndex = UINT32_MAX;
    }
    m_pPoolRTV = nullptr;
}
