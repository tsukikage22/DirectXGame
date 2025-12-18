#pragma once

#include <DirectXTK12/ResourceUploadBatch.h>

#include <array>
#include <memory>
#include <optional>

#include "Engine/ConstantBuffer.h"
#include "Engine/MaterialSrvTable.h"
#include "Engine/ModelAsset.h"
#include "Engine/ShaderConstants.h"
#include "Engine/TextureManager.h"

class DescriptorPool;

enum class TextureUsage : uint32_t {
    BaseColor = 0,
    MetallicRoughness,
    Normal,
    Emissive,
    Occlusion,
    Count  // = 5
};

class MaterialGPU {
public:
    MaterialGPU();
    ~MaterialGPU();

    /// @brief 初期化処理，MaterialAssetからGPUリソースを作成
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        DescriptorPool* pPoolSRV, TextureManager* pTextureManager,
        const MaterialAsset& materialAsset);

    /// @brief 終了処理，リソースの解放
    void Term();

    //========================================
    // アクセサ
    //========================================
    /// @brief GPUディスクリプタハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferHandle() const {
        return m_constantBuffer.GetGPUHandle();
    }

    /// @brief GPU仮想アドレスの取得
    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress() const {
        return m_constantBuffer.GetGPUVirtualAddress();
    }

    /// @brief 描画で使うためのテクスチャを取得する
    /// @param usage テクスチャの用途（baseColor, metallicなど）
    /// @return テクスチャのポインタ
    ShaderResourceTexture* GetTexture(TextureUsage usage) const;

    /// @brief 指定した用途のテクスチャハンドルを取得する
    std::optional<uint32_t> GetTextureHandle(TextureUsage usage) const;

    /// @brief SRVテーブルの先頭GPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvTableBaseGPUHandle() const;

private:
    ConstantBuffer m_constantBuffer;        // マテリアル定数
    shader::MaterialConstants m_constants;  // 定数バッファ用データ
    MaterialSrvTable m_srvTable;            // マテリアル用SRVテーブル

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