#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"

class ColorTarget {
public:
    ColorTarget();
    ~ColorTarget();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief バックバッファを参照してRTVを作成する
    /// @param pDevice デバイス
    /// @param pPoolRTV RTV用ディスクリプタプール
    /// @param index バックバッファのインデックス
    /// @param pSwapChain スワップチェーン
    /// @return 成功した場合はtrueを返す
    ///////////////////////////////////////////////////////////////////////////
    bool InitFromBackBuffer(ID3D12Device* pDevice, DescriptorPool* pPoolRTV,
        uint32_t index, IDXGISwapChain* pSwapChain);

    ///////////////////////////////////////////////////////////////////////////
    /// @brief リソースの解放
    ///////////////////////////////////////////////////////////////////////////
    void Term();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief RTVインデックスの取得
    /// @return RTVインデックス
    ///////////////////////////////////////////////////////////////////////////
    uint32_t GetRTVIndex() const { return m_RTVIndex; }

private:
    engine::ComPtr<ID3D12Resource> m_pTarget;  // リソース
    DescriptorPool* m_pPoolRTV;                // ディスクリプタプール
    uint32_t m_RTVIndex;                       // RTVインデックス
    D3D12_RENDER_TARGET_VIEW_DESC m_ViewDesc;  // RTVのディスクリプタ

    ColorTarget(const ColorTarget&) = delete;
    void operator=(const ColorTarget&) = delete;
};