#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Resource/TextureResource.h"

// 前方宣言
class DescriptorPool;

class DepthTarget {
public:
    DepthTarget();
    ~DepthTarget();

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

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const {
        return m_DSVAllocation.GetCPUHandle();
    }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const {
        return m_DSVAllocation.GetGPUHandle();
    }

private:
    TextureResource m_Target;                  // リソース
    DescriptorPool* m_pPoolDSV;                // ディスクリプタプール
    DescriptorAllocation m_DSVAllocation;      // DSVのディスクリプタ
    D3D12_DEPTH_STENCIL_VIEW_DESC m_ViewDesc;  // DSVのディスクリプタ

    DepthTarget(const DepthTarget&)    = delete;
    void operator=(const DepthTarget&) = delete;
};