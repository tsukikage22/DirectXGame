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

    bool LoadFromFile(
        const std::filesystem::path& path, std::vector<MeshAsset>& outMeshes);

private:
    static Assimp::Importer m_importer;

    bool ParseMesh(const aiMesh* mesh, MeshAsset& outMesh);
};
