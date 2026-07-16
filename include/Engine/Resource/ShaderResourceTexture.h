#pragma once

#include <d3d12.h>
#include <directxtex.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <optional>
#include <vector>

#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Resource/TextureResource.h"
#include "Engine/Model/ModelAsset.h"

class DescriptorPool;

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

    /// @brief 終了処理（SRV解放，Resource解放）
    void Term();

    //=======================================
    // アクセサ
    //=======================================
    const TextureResource* GetTextureResource() const { return &m_texture; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetDefaultSrvGpu() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultSrvCpu() const;

private:
    TextureResource m_texture;   // テクスチャリソース
    DescriptorPool* m_pPoolSRV;  // SRV用ディスクリプタプール

    // 一つのテクスチャに対して複数のSRVを作ることを想定し，配列にする
    std::vector<DescriptorAllocation> m_srvs;  // SRVインデックス

    // コピー禁止
    ShaderResourceTexture(const ShaderResourceTexture&)            = delete;
    ShaderResourceTexture& operator=(const ShaderResourceTexture&) = delete;
};
