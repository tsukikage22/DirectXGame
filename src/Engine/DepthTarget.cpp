#include "Engine/DepthTarget.h"

DepthTarget::DepthTarget()
    : m_Target(), m_pPoolDSV(nullptr), m_DSVIndex(UINT32_MAX), m_ViewDesc{} {}

DepthTarget::~DepthTarget() { Term(); }

bool DepthTarget::Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV,
    uint32_t width, uint32_t height, DXGI_FORMAT format) {
    // 引数チェック
    if (!pDevice || !pPoolDSV || width == 0 || height == 0) {
        return false;
    }

    D3D12_CLEAR_VALUE clearValue    = {};
    clearValue.Format               = format;
    clearValue.DepthStencil.Depth   = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // リソースの生成
    if (!m_Target.InitAsTexture2D(pDevice, width, height, format, 1,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue)) {
        return false;
    }

    // DSVの作成
    m_pPoolDSV = pPoolDSV;
    m_DSVIndex = m_pPoolDSV->Allocate();

    m_ViewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Format             = format;
    m_ViewDesc.Texture2D.MipSlice = 0;
    m_ViewDesc.Flags              = D3D12_DSV_FLAG_NONE;

    pDevice->CreateDepthStencilView(m_Target.GetResource(), &m_ViewDesc,
        m_pPoolDSV->GetCPUHandle(m_DSVIndex));

    return true;
}

void DepthTarget::Term() {
    m_Target.Term();

    if (m_pPoolDSV != nullptr && m_DSVIndex != UINT32_MAX) {
        m_pPoolDSV->Free(m_DSVIndex);
        m_DSVIndex = UINT32_MAX;
    }
    m_pPoolDSV = nullptr;
}