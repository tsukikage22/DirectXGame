#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"

class DepthStencil {
public:
    DepthStencil();
    ~DepthStencil();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief 深度ステンシルバッファの初期化
    /// @param pDevice デバイス
    /// @param pPoolDSV DSV用ディスクリプタプール
    /// @param width 幅
    /// @param height 高さ
    /// @return 成功した場合はtrueを返す
    ///////////////////////////////////////////////////////////////////////////
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV, uint32_t width,
        uint32_t height, DXGI_FORMAT format);

    ///////////////////////////////////////////////////////////////////////////
    /// @brief リソースの解放
    ///////////////////////////////////////////////////////////////////////////
    void Term();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief DSVインデックスの取得
    /// @return DSVインデックス
    ///////////////////////////////////////////////////////////////////////////
    uint32_t GetDSVIndex() const { return m_DSVIndex; }

private:
    engine::ComPtr<ID3D12Resource> m_pTarget;  // リソース
    DescriptorPool* m_pPoolDSV;                // ディスクリプタプール
    uint32_t m_DSVIndex;                       // DSVインデックス
    D3D12_DEPTH_STENCIL_VIEW_DESC m_ViewDesc;  // DSVのディスクリプタ

    DepthStencil(const DepthStencil&) = delete;
    void operator=(const DepthStencil&) = delete;
};