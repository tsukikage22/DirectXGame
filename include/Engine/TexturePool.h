#pragma once

#include <d3d12.h>
#include <directxtex.h>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"
#include "Engine/TextureGPU.h"

class TexturePool {
public:
    TexturePool();
    ~TexturePool() { Term(); }

    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV);

    void Term();

    /// @brief ImageAsset配列からテクスチャを生成
    /// @param images 生成するテクスチャの元となるImageAsset配列
    /// @param batch リソースアップロードバッチ
    /// @return 成功した場合はtrueを返す
    bool CreateFromImages(const std::vector<ImageAsset>& images,
        DirectX::ResourceUploadBatch& batch);

    /// @brief デフォルトテクスチャの生成
    /// @param batch ResourceUploadBatch
    /// @return 成功でtrue
    bool CreateDefaultTexture(DirectX::ResourceUploadBatch& batch);

    /// @brief 指定したインデックスのテクスチャを取得
    /// @param index 取得するテクスチャのインデックス
    /// @return 指定したテクスチャのポインタ
    TextureGPU* GetTexture(int index);

private:
    ID3D12Device* m_pDevice;     // デバイス
    DescriptorPool* m_pPoolSRV;  // SRV用ディスクリプタプール
    std::vector<std::unique_ptr<TextureGPU>> m_textures;  // テクスチャ配列
    std::unique_ptr<TextureGPU> m_pDefaultTexture;  // デフォルトテクスチャ

    // コピー禁止
    TexturePool(const TexturePool&)            = delete;
    TexturePool& operator=(const TexturePool&) = delete;
};