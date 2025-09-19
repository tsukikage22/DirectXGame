/// @file Mesh.h
/// @brief GPUが扱うメッシュデータ
#pragma once

#include <vector>

#include "Engine/VertexTypes.h"

struct MeshAsset {
    std::vector<StandardVertex> vertices;  // 頂点データ
    std::vector<uint16_t> indices;         // インデックスデータ
    uint32_t materialID = 0;               // マテリアルインデックス
};
