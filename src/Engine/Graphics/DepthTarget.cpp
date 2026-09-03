#include "Engine/Graphics/DepthTarget.h"

#include <cassert>

#include "Engine/Core/DescriptorPool.h"

namespace /*anonymous*/ {

void BuildDSV(ID3D12Device* pDevice, ID3D12Resource* pResource,
    DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle) {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format                        = format;
    dsvDesc.Texture2D.MipSlice            = 0;
    dsvDesc.Flags                         = D3D12_DSV_FLAG_NONE;
    pDevice->CreateDepthStencilView(pResource, &dsvDesc, dsvHandle);
}

void BuildSRV(ID3D12Device* pDevice, ID3D12Resource* pResource,
    DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = format;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;
    pDevice->CreateShaderResourceView(pResource, &srvDesc, srvHandle);
}

}  // namespace

DepthTarget::DepthTarget()
    : m_Target(), m_pPoolDSV(nullptr), m_pPoolSRV(nullptr) {}

DepthTarget::~DepthTarget() { Term(); }

bool DepthTarget::Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV,
    DescriptorPool* pPoolSRV, uint32_t width, uint32_t height,
    DXGI_FORMAT format) {
    // 引数チェック
    if (!pDevice || !pPoolDSV || width == 0 || height == 0) {
        return false;
    }

    // リソースのR32_TYPELESSとSRVのR32_FLOATはハードコードしているので，
    // DSVのフォーマットはD32_FLOATのみ対応する
    // TODO:将来的に他のフォーマットに対応する場合は，ハードコーティングをやめて分岐を作る
    if (format != DXGI_FORMAT_D32_FLOAT) {
        OutputDebugStringW(
            L"DepthTarget only supports DXGI_FORMAT_D32_FLOAT format.\n");
        return false;
    }

    D3D12_CLEAR_VALUE clearValue    = {};
    clearValue.Format               = format;
    clearValue.DepthStencil.Depth   = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // リソースの生成
    if (!m_Target.InitAsTexture2D(pDevice, width, height,
            DXGI_FORMAT_R32_TYPELESS,  // TYPELESSで作らないとSRVでアクセスできない
            1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue)) {
        return false;
    }

    // DSVの作成
    m_pPoolDSV      = pPoolDSV;
    m_DSVAllocation = m_pPoolDSV->Allocate();
    BuildDSV(pDevice, m_Target.GetResource(), format,
        m_DSVAllocation.GetCPUHandle());

    // SRVの作成
    if (pPoolSRV) {
        m_pPoolSRV      = pPoolSRV;
        m_SRVAllocation = m_pPoolSRV->Allocate();
        // 深度バッファのSRVはR32_FLOATでアクセスする
        BuildSRV(pDevice, m_Target.GetResource(), DXGI_FORMAT_R32_FLOAT,
            m_SRVAllocation.GetCPUHandle());
    }

    // 幅と高さの保持
    m_width  = width;
    m_height = height;

    return true;
}

void DepthTarget::Term() {
    m_width  = 0;
    m_height = 0;
    m_Target.Term();
    m_DSVAllocation = DescriptorAllocation{};
    m_SRVAllocation = DescriptorAllocation{};
    m_pPoolDSV      = nullptr;
    m_pPoolSRV      = nullptr;
}

D3D12_VIEWPORT DepthTarget::MakeViewport() const {
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX       = 0.0f;
    viewport.TopLeftY       = 0.0f;
    viewport.Width          = static_cast<float>(m_width);
    viewport.Height         = static_cast<float>(m_height);
    viewport.MinDepth       = 0.0f;
    viewport.MaxDepth       = 1.0f;
    return viewport;
}

D3D12_RECT DepthTarget::MakeScissorRect() const {
    D3D12_RECT rect = {};
    rect.left       = 0;
    rect.top        = 0;
    rect.right      = static_cast<LONG>(m_width);
    rect.bottom     = static_cast<LONG>(m_height);
    return rect;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthTarget::GetDSVCPUHandle() const {
    if (!m_DSVAllocation.IsValid()) {
        return {};
    }
    return m_DSVAllocation.GetCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthTarget::GetSRVCPUHandle() const {
    if (!m_SRVAllocation.IsValid()) {
        return {};
    }
    return m_SRVAllocation.GetCPUHandle();
}

D3D12_GPU_DESCRIPTOR_HANDLE DepthTarget::GetSRVGPUHandle() const {
    if (!m_SRVAllocation.IsValid()) {
        return {};
    }
    return m_SRVAllocation.GetGPUHandle();
}