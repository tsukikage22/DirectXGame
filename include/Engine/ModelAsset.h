/// @file Mesh.h
/// @brief CPUが扱うメッシュデータ
#pragma once

#include <string>
#include <vector>

#include "Engine/VertexTypes.h"

/// @brief CPU側メッシュデータ
struct MeshAsset {
    std::vector<StandardVertex> vertices;  // 頂点データ
    std::vector<uint32_t> indices;         // インデックスデータ
    uint32_t materialID = 0;               // マテリアルID
};

/// @brief CPU側画像データ
struct ImageAsset {
    std::vector<uint8_t> imageData;  // GLBの埋め込み画像データ
    std::string format;              // tex->achFormatの文字列，"jpg"や"png"など
    bool isSRGB = false;             // sRGBとして扱うかどうか

    bool IsValid() const { return !imageData.empty(); }
};

/// @brief グローバルに有効なテクスチャのハンドル
struct TextureHandle {
    // assimpのインデックスはscene内でしか意味がないので使わない
    // TextureManagerが内部配列に追加した際に設定する
    // 全テクスチャに与えられるID，UINT32_MAXなら無効
    uint32_t index = UINT32_MAX;
    bool IsValid() const { return index != UINT32_MAX; }
};

/// @brief CPUのメモリ上に保持されるマテリアルデータ
struct MaterialAsset {
    std::wstring name;

    // パラメータ
    DirectX::XMFLOAT4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallicFactor              = 1.0f;
    float roughnessFactor             = 1.0f;
    DirectX::XMFLOAT3 emissiveFactor  = { 0.0f, 0.0f, 0.0f };
    float occlusionFactor             = 1.0f;

    // テクスチャへの参照（TextureManager内配列のインデックス）
    TextureHandle baseColorTexture;          // base color
    TextureHandle metallicRoughnessTexture;  // metallic-roughness
    TextureHandle occlusionTexture;          // occlusion
    TextureHandle normalTexture;             // normal
    TextureHandle emissiveTexture;           // emissive

    // モデルロード時の一時的なインデックス
    int baseColorLocalTextureIndex         = -1;
    int metallicRoughnessLocalTextureIndex = -1;
    int occlusionLocalTextureIndex         = -1;
    int normalLocalTextureIndex            = -1;
    int emissiveLocalTextureIndex          = -1;
};

/// @brief モデル全体のデータ
struct ModelAsset {
    std::string name;

    std::vector<MeshAsset> meshes;
    std::vector<MaterialAsset> materials;
    std::vector<ImageAsset> images;

    bool IsValid() const { return !meshes.empty(); }
};
