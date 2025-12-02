#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include <cassert>
#include <optional>

#include "Engine/ComPtr.h"

/// @brief GPU上のテクスチャリソース
class TextureResource {
public:
    TextureResource();
    ~TextureResource();

    // ムーブコンストラクタ・ムーブ代入
    TextureResource(TextureResource&&) noexcept            = default;
    TextureResource& operator=(TextureResource&&) noexcept = default;

    /// @brief スワップチェーンからリソースを初期化
    /// @param pSwapChain
    /// @param bufferIndex
    /// @return
    bool InitFromSwapChain(IDXGISwapChain* pSwapChain, UINT bufferIndex);

    /// @brief 新規テクスチャをDEFAULTヒープ上に作成
    /// @return
    bool InitAsTexture2D(ID3D12Device* pDevice, UINT width, UINT height,
        DXGI_FORMAT format, UINT mipLevels, D3D12_RESOURCE_FLAGS flags,
        D3D12_RESOURCE_STATES initState,
        const D3D12_CLEAR_VALUE* pClearValue = nullptr);

    /// @brief リソースの解放
    void Term();

    //=======================================
    // アクセサ
    //=======================================
    ID3D12Resource* GetResource() const { return m_pResource.Get(); }

    D3D12_RESOURCE_DESC GetDesc() const {
        if (!m_pResource) {
            assert(false && "Resource is not initialized");
        }
        return m_pResource->GetDesc();
    }

private:
    engine::ComPtr<ID3D12Resource> m_pResource;  // テクスチャリソース本体
    uint32_t m_width;                            // テクスチャ幅
    uint32_t m_height;                           // テクスチャ高さ
    uint32_t m_mipLevels;                        // ミップレベル数

    TextureResource(const TextureResource&)            = delete;
    TextureResource& operator=(const TextureResource&) = delete;
};
