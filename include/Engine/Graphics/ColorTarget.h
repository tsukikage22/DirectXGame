#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Core/DescriptorPool.h"
#include "Engine/Resource/TextureResource.h"

class ColorTarget {
public:
    ColorTarget();
    ~ColorTarget();

    /// @brief バッファを参照してRTVを作成する
    /// @param pDevice デバイス
    /// @param pPoolRTV RTV用ディスクリプタプール
    /// @param index バックバッファのインデックス
    /// @param pSwapChain スワップチェーン
    /// @return 成功した場合はtrueを返す
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, uint32_t index,
        IDXGISwapChain* pSwapChain);

    /// @brief オフスクリーン用のRTVを作成する
    /// @param pDevice デバイス
    /// @param pPoolRTV RTV用ディスクリプタプール
    /// @param pPoolSRV SRV用ディスクリプタプール
    /// @param width 幅
    /// @param height 高さ
    /// @param format フォーマット
    /// @return 成功した場合はtrueを返す
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV,
        DescriptorPool* pPoolSRV, uint32_t width, uint32_t height,
        DXGI_FORMAT format);

    /// @brief リソースの解放
    void Term();

    //========================================================================
    // アクセサ
    //========================================================================
    ID3D12Resource* GetResource() const { return m_Target.GetResource(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUHandle() const {
        return m_RTVAllocation.GetCPUHandle();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const {
        if (!m_SRVAllocation.IsValid()) {
            return {};
        }
        return m_SRVAllocation.GetCPUHandle();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle() const {
        if (!m_SRVAllocation.IsValid()) {
            return {};
        }
        return m_SRVAllocation.GetGPUHandle();
    }

private:
    TextureResource m_Target;                  // リソース
    DescriptorAllocation m_RTVAllocation;      // RTVのディスクリプタ
    DescriptorAllocation m_SRVAllocation;      // SRVのディスクリプタ
    D3D12_RENDER_TARGET_VIEW_DESC m_ViewDesc;  // RTVのディスクリプタ

    ColorTarget(const ColorTarget&)    = delete;
    void operator=(const ColorTarget&) = delete;
};