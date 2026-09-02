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

    /// @brief 深度バッファの初期化（シャドウマップの場合SRVをもたせる）
    /// @param pDevice  デバイス
    /// @param pPoolDSV DSVプール
    /// @param pPoolSRV SRVプール
    /// @param width    幅
    /// @param height   高さ
    /// @param format   DSVのフォーマット
    /// @return 成功した場合はtrueを返す
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV,
        DescriptorPool* pPoolSRV, uint32_t width, uint32_t height,
        DXGI_FORMAT format);

    /// @brief リソースの解放
    void Term();

    /// @brief このターゲット全体を覆うビューポートを作成する
    D3D12_VIEWPORT MakeViewport() const;

    /// @brief このターゲット全体を覆うシザー矩形を作成する
    D3D12_RECT MakeScissorRect() const;

    //=======================================
    // アクセサ
    //=======================================
    ID3D12Resource* GetResource() const { return m_Target.GetResource(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUHandle() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle() const;

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

private:
    TextureResource m_Target;    // リソース
    DescriptorPool* m_pPoolDSV;  // DSVプール
    DescriptorPool* m_pPoolSRV;  // SRVプール

    DescriptorAllocation m_DSVAllocation;  // DSVのディスクリプタ
    DescriptorAllocation m_SRVAllocation;  // SRVのディスクリプタ

    uint32_t m_width  = 0;  // 幅
    uint32_t m_height = 0;  // 高さ

    DepthTarget(const DepthTarget&)    = delete;
    void operator=(const DepthTarget&) = delete;
};