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

    ///////////////////////////////////////////////////////////////////////////
    /// @brief バッファを参照してRTVを作成する
    /// @param pDevice デバイス
    /// @param pPoolRTV RTV用ディスクリプタプール
    /// @param index バックバッファのインデックス
    /// @param pSwapChain スワップチェーン
    /// @return 成功した場合はtrueを返す
    ///////////////////////////////////////////////////////////////////////////
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, uint32_t index,
        IDXGISwapChain* pSwapChain);

    ///////////////////////////////////////////////////////////////////////////
    /// @brief リソースの解放
    ///////////////////////////////////////////////////////////////////////////
    void Term();

    //========================================================================
    // アクセサ
    //========================================================================
    ID3D12Resource* GetResource() const { return m_Target.GetResource(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const {
        return m_RTVAllocation.GetCPUHandle();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const {
        return m_RTVAllocation.GetGPUHandle();
    }

private:
    TextureResource m_Target;                  // リソース
    DescriptorPool* m_pPoolRTV;                // ディスクリプタプール
    DescriptorAllocation m_RTVAllocation;      // RTVのディスクリプタ
    D3D12_RENDER_TARGET_VIEW_DESC m_ViewDesc;  // RTVのディスクリプタ

    ColorTarget(const ColorTarget&)    = delete;
    void operator=(const ColorTarget&) = delete;
};