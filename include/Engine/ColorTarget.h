#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"
#include "Engine/TextureResource.h"

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

    ///////////////////////////////////////////////////////////////////////////
    /// @brief RTVのディスクリプタプールインデックスの取得
    /// @return RTVインデックス
    ///////////////////////////////////////////////////////////////////////////
    uint32_t GetRTVIndex() const { return m_RTVIndex; }

private:
    TextureResource m_Target;                  // リソース
    DescriptorPool* m_pPoolRTV;                // ディスクリプタプール
    uint32_t m_RTVIndex;                       // RTVインデックス
    D3D12_RENDER_TARGET_VIEW_DESC m_ViewDesc;  // RTVのディスクリプタ

    ColorTarget(const ColorTarget&)    = delete;
    void operator=(const ColorTarget&) = delete;
};