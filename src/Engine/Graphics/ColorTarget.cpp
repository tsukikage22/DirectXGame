#include "Engine/Graphics/ColorTarget.h"

ColorTarget::ColorTarget()
    : m_Target(), m_RTVAllocation(), m_SRVAllocation(), m_ViewDesc{} {}

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
    m_RTVAllocation = pPoolRTV->Allocate();

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

// オフスクリーン用RTVの作成
bool ColorTarget::Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV,
    DescriptorPool* pPoolSRV, uint32_t width, uint32_t height,
    DXGI_FORMAT format) {
    // 引数チェック
    if (pDevice == nullptr || pPoolRTV == nullptr || pPoolSRV == nullptr ||
        width == 0 || height == 0) {
        return false;
    }

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format            = format;
    // 黒，透明で初期化する
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 0.0f;

    // テクスチャリソースの作成
    if (!m_Target.InitAsTexture2D(pDevice, width, height, format, 1,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue)) {
        return false;
    }

    // RTVの作成
    m_RTVAllocation                 = pPoolRTV->Allocate();
    m_ViewDesc.Format               = format;
    m_ViewDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_ViewDesc.Texture2D.MipSlice   = 0;
    m_ViewDesc.Texture2D.PlaneSlice = 0;
    pDevice->CreateRenderTargetView(
        m_Target.GetResource(), &m_ViewDesc, m_RTVAllocation.GetCPUHandle());

    // SRVの作成
    m_SRVAllocation                         = pPoolSRV->Allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = format;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;
    pDevice->CreateShaderResourceView(
        m_Target.GetResource(), &srvDesc, m_SRVAllocation.GetCPUHandle());

    return true;
}

void ColorTarget::Term() {
    m_RTVAllocation = DescriptorAllocation{};
    m_SRVAllocation = DescriptorAllocation{};
    m_Target.Term();
}
