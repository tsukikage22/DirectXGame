#pragma once

#include <directxtk12/ResourceUploadBatch.h>
#include <directxtk12/WICTextureLoader.h>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"

/// @brief GPU上の個別のテクスチャリソース
class TextureGPU {
public:
    TextureGPU() = default;
    ~TextureGPU() { Term(); }

    /// @brief ImageAssetからテクスチャを生成する
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
        DirectX::ResourceUploadBatch& resourceUploadBatch,
        const ImageAsset& imageAsset);

    /// @brief 作成済みリソースから初期化（デフォルトテクスチャを想定）
    bool InitFromResource(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
        ID3D12Resource* pResource);

    /// @brief 終了処理，リソースの開放
    void Term();

    /// @brief SRVのGPUハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

    ID3D12Resource* GetResource() const { return m_pTexture.Get(); }

private:
    engine::ComPtr<ID3D12Resource> m_pTexture;  // テクスチャリソース
    DescriptorPool* m_pPool = nullptr;          // ディスクリプタプール
    uint32_t m_index        = 0;  // ディスクリプタプールのインデックス

    // コピー禁止
    TextureGPU(const TextureGPU&)            = delete;
    TextureGPU& operator=(const TextureGPU&) = delete;
};