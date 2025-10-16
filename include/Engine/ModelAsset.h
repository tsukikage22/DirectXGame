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

    bool IsValid() const { return !imageData.empty(); }
};

struct TextureReference {
    // ImageAsset配列のインデックス，-1なら無効
    // モデルの読み込み時に設定する，
    // リソースの作成は配列のインデックス順に行うためTexturePoolのインデックスと一致し，
    // MaterialGPUのインデックスとしても使える
    int index = -1;
    bool IsValid() const { return index >= 0; }
};

/// @brief CPUのメモリ上に保持されるマテリアルデータ
struct MaterialAsset {
    std::wstring name;

    // パラメータ
    DirectX::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallic              = 0.0f;
    float roughness             = 0.5f;

    // テクスチャへの参照（ImageAsset配列へのインデックス）
    TextureReference baseColorTexture;  // base color
    TextureReference metallicTexture;   // metallic
    TextureReference roughnessTexture;  // roughness
    TextureReference normalTexture;     // normal
};

/// @brief モデル全体のデータ
struct ModelAsset {
    std::string name;

    std::vector<MeshAsset> meshes;
    std::vector<MaterialAsset> materials;
    std::vector<ImageAsset> images;

    bool IsValid() const { return !meshes.empty(); }
};
