/// @file GLBImporter.h
/// @brief GLBファイルの読み込み

#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <filesystem>
#include <locale>
#include <string>
#include <vector>

#include "Engine/ModelAsset.h"

class GLBImporter {
public:
    GLBImporter()  = delete;
    ~GLBImporter() = delete;

    /// @brief glbファイルを読み込み，ModelAssetを出力
    /// @param path ファイルパス
    /// @param[out] outModel 出力先
    /// @return 成功時true
    static bool LoadFromFile(
        const std::filesystem::path& path, ModelAsset& outModel);

private:
    static bool ParseMesh(const aiMesh* mesh, MeshAsset& outMesh);
    static bool ParseMaterial(
        const aiMaterial* material, int imageCount, MaterialAsset& outMaterial);
};
