#pragma once

#include <DirectXTK12/ResourceUploadBatch.h>

#include <array>
#include <memory>
#include <optional>

#include "Engine/ConstantBuffer.h"
#include "Engine/DescriptorPool.h"
#include "Engine/ModelAsset.h"
#include "Engine/ShaderConstants.h"
#include "Engine/TextureManager.h"

class MaterialGPU {
public:
    enum class TextureUsage : uint32_t {
        BaseColor = 0,
        MetallicRoughness,
        Normal,
        Emissive,
        Occlusion,
        Count  // = 5
    };

    MaterialGPU();
    ~MaterialGPU();

    /// @brief 初期化処理，MaterialAssetからGPUリソースを作成
    /// @return
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        TextureManager* pTextureManager, const MaterialAsset& materialAsset);

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
    ShaderResourceTexture* GetTexture(TextureUsage usage) const;

private:
    ConstantBuffer m_constantBuffer;        // マテリアル定数
    shader::MaterialConstants m_constants;  // 定数バッファ用データ

    // テクスチャ
    TextureManager* m_pTextureManager;  // テクスチャマネージャ
    std::optional<uint32_t>
        m_baseColorIndex;  // ベースカラーテクスチャのインデックス
    std::optional<uint32_t>
        m_metallicRoughnessIndex;  // メタリックテクスチャのインデックス
    std::optional<uint32_t>
        m_occlusionIndex;  // オクルージョンテクスチャのインデックス
    std::optional<uint32_t> m_normalIndex;  // 法線テクスチャのインデックス
    std::optional<uint32_t>
        m_emissiveIndex;  // エミッシブテクスチャのインデックス

    // コピー禁止
    MaterialGPU(const MaterialGPU&)            = delete;
    MaterialGPU& operator=(const MaterialGPU&) = delete;
};