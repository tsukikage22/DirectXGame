/// @file FBXLoader.h
/// @brief assimpによるFBXファイルの読み込み
#pragma once

#include <DirectXMath.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <filesystem>
#include <vector>

#include "Engine/AssetPath.h"
#include "Engine/MeshAsset.h"
#include "Engine/VertexTypes.h"

class FBXImporter {
public:
    FBXImporter()  = delete;
    ~FBXImporter() = delete;

    static bool LoadFromFile(const std::filesystem::path& path,
        std::vector<MeshAsset>& outMeshes,
        std::vector<MaterialAsset>& outMaterials);

private:
    static bool ParseMesh(const aiMesh* mesh, MeshAsset& outMesh);

    static bool ParseMaterial(
        const aiMaterial* material, MaterialAsset& outMaterial);
};
