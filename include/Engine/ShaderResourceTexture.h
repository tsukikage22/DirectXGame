#pragma once

#include <d3d12.h>
#include <directxtex.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <optional>
#include <vector>

#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"
#include "Engine/TextureResource.h"

/// @brief SRVインデックス
struct SrvIndex {
    uint32_t index = UINT32_MAX;
    bool IsValid() const { return index != UINT32_MAX; }
};

/// @brief 個別のテクスチャリソースとそのSRVの管理
class ShaderResourceTexture {
public:
    ShaderResourceTexture();
    ~ShaderResourceTexture();

    // ムーブコンストラクタ・ムーブ代入
    ShaderResourceTexture(ShaderResourceTexture&&) noexcept = default;
    ShaderResourceTexture& operator=(
        ShaderResourceTexture&&) noexcept = default;

    /// @brief テクスチャリソースとデフォルトSRVの作成
    bool InitFromImage(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
        const ImageAsset& image, DirectX::ResourceUploadBatch& batch);

    bool InitSolidColorRGBA8(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
        uint32_t color, DirectX::ResourceUploadBatch& batch);

    // TODO: Init関数の共通部分をまとめてヘルパ関数にする

    /// @brief 終了処理（SRV解放，Resource解放）
    void Term();

    //=======================================
    // アクセサ
    //=======================================
    /// @brief デフォルトSRVのインデックス
    SrvIndex GetDefaultSrvIndex() const;

    const TextureResource* GetTextureResource() const { return &m_Texture; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetDefaultSrvGpu() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultSrvCpu() const;

private:
    TextureResource m_Texture;     // テクスチャリソース
    DescriptorPool* m_pPoolSRV;    // SRV用ディスクリプタプール
    std::vector<SrvIndex> m_srvs;  // SRVインデックス

    // コピー禁止
    ShaderResourceTexture(const ShaderResourceTexture&)            = delete;
    ShaderResourceTexture& operator=(const ShaderResourceTexture&) = delete;
};
