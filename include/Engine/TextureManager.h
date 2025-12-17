#pragma once

#include <d3d12.h>
#include <directxtex.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <memory>
#include <optional>
#include <vector>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"
#include "Engine/ShaderResourceTexture.h"
#include "Engine/TextureResource.h"

/// @brief テクスチャの所有とハンドル管理
class TextureManager {
public:
    TextureManager();
    ~TextureManager() { Term(); }

    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV);

    void Term();

    /// @brief
    /// ModelAssetからのテクスチャ構築とマテリアル内テクスチャハンドルの解決
    void BuildTexturesFromModelAsset(
        ModelAsset& modelAsset, DirectX::ResourceUploadBatch& batch);

    //=========================================
    // デフォルトテクスチャの取得
    //==========================================
    ShaderResourceTexture* GetWhiteDefault() const;
    ShaderResourceTexture* GetNormalFlat() const;
    ShaderResourceTexture* GetRmaDefault() const;

    //=========================================
    // Factory メソッド
    //=========================================
    /// @brief ImageAssetからテクスチャを生成
    /// @return 生成したテクスチャのインデックス
    uint32_t CreateFromImageAsset(
        const ImageAsset& image, DirectX::ResourceUploadBatch& batch);

    /// @brief 単色テクスチャの生成
    bool CreateSolidColorTexture(DirectX::ResourceUploadBatch& batch,
        uint32_t color, ShaderResourceTexture& outTexture);

    /// @brief デフォルトテクスチャすべての生成
    bool CreateDefaultTextures(DirectX::ResourceUploadBatch& batch);

    //=========================================
    // TextureResourceの管理
    //=========================================
    /// @brief 指定したインデックスのテクスチャを取得
    /// @param index 取得するテクスチャのインデックス
    ShaderResourceTexture* GetTexture(uint32_t index);

    /// @brief テクスチャ数の取得
    size_t GetTextureCount() const { return m_textures.size(); }

    /// @brief SRVハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGPUHandle(TextureHandle handle) const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpuHandle(TextureHandle handle) const;

private:
    ID3D12Device* m_pDevice;                        // デバイス
    DescriptorPool* m_pPoolSRV;                     // SRV用ディスクリプタプール
    std::vector<ShaderResourceTexture> m_textures;  // テクスチャプール

    // デフォルトテクスチャ
    std::unique_ptr<ShaderResourceTexture> m_pDefaultWhiteTexture;
    std::unique_ptr<ShaderResourceTexture> m_pDefaultNormalFlatTexture;
    std::unique_ptr<ShaderResourceTexture> m_pDefaultRmaTexture;

    // private methods

    // コピー禁止
    TextureManager(const TextureManager&)            = delete;
    TextureManager& operator=(const TextureManager&) = delete;
};
