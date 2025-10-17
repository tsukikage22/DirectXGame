#pragma once

#include <ResourceUploadBatch.h>

#include <array>
#include <memory>

#include "Engine/ConstantBuffer.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"
#include "Engine/ShaderConstants.h"
#include "Engine/TexturePool.h"

class MaterialGPU {
public:
    enum class TextureUsage : uint32_t {
        BaseColor = 0,
        Metallic,
        Roughness,
        Normal,
        Count  // = 4
    };

    MaterialGPU();
    ~MaterialGPU();

    /// @brief 初期化処理，MaterialAssetからGPUリソースを作成
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        TexturePool* pTexturePool, const MaterialAsset& materialAsset);

    /// @brief 終了処理，リソースの解放
    void Term();

    /// @brief マテリアル定数用の定数バッファのGPUハンドルを返す
    /// @return 定数バッファのGPUハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferHandle() const {
        return m_constantBuffer.GetGPUHandle();
    }

    /// @brief 描画で使うためのテクスチャを取得する
    /// @param usage テクスチャの用途（baseColor, metallicなど）
    /// @return テクスチャのポインタ
    TextureGPU* GetTexture(TextureUsage usage) const;

private:
    ConstantBuffer m_constantBuffer;        // マテリアル定数
    shader::MaterialConstants m_constants;  // 定数バッファ用データ

    // テクスチャ
    TexturePool* m_pTexturePool;  // テクスチャプール
    int m_baseColorIndex = -1;    // ベースカラーテクスチャのインデックス
    int m_metallicIndex  = -1;    // メタリックテクスチャのインデックス
    int m_roughnessIndex = -1;    // ラフネステクスチャのインデックス
    int m_normalIndex    = -1;    // 法線テクスチャのインデックス

    // コピー禁止
    MaterialGPU(const MaterialGPU&)            = delete;
    MaterialGPU& operator=(const MaterialGPU&) = delete;
};