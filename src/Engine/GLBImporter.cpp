/// @file GLBImporter.cpp
/// @brief GLBファイルの読み込み

#include "Engine/GLBImporter.h"

bool GLBImporter::LoadFromFile(
    const std::filesystem::path& path, ModelAsset& outModel) {
    // ファイルパスの確認
    if (!std::filesystem::exists(path)) {
        OutputDebugStringW(L"Error: File not found.\n");
        return false;
    }

    // 出力先の初期化
    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.images.clear();

    Assimp::Importer importer;
    unsigned int flags = 0;
    flags |= aiProcess_Triangulate |               // 三角形化
             aiProcess_PreTransformVertices |      // 変換の適用
             aiProcess_GenSmoothNormals |          // スムース法線ベクトル生成
             aiProcess_CalcTangentSpace |          // 接線ベクトル計算
             aiProcess_GenUVCoords |               // UV座標生成
             aiProcess_RemoveRedundantMaterials |  // 冗長なマテリアルの削除
             aiProcess_OptimizeMeshes;             // メッシュの最適化

    const aiScene* scene = importer.ReadFile(path.string(), flags);

    if (!scene) {
        OutputDebugStringW(L"Error: Scene is null.\n");
        return false;
    }
    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        OutputDebugStringW(L"Error: Scene is incomplete.\n");
        return false;
    }
    if (!scene->mRootNode || !scene->HasMeshes()) {
        OutputDebugStringW(L"Error: Scene invalid.\n");
        return false;
    }

    // テクスチャの読み込み
    outModel.images.reserve(scene->mNumTextures);
    for (int i = 0; i < scene->mNumTextures; i++) {
        const aiTexture* texture = scene->mTextures[i];
        if (!texture) {
            continue;
        }

        ImageAsset imageAsset;

        if (texture->mHeight == 0) {
            // 圧縮画像の読み込み
            const uint8_t* src = reinterpret_cast<uint8_t*>(texture->pcData);
            const size_t size  = static_cast<size_t>(texture->mWidth);
            imageAsset.imageData.assign(src, src + size);

            // formatは小文字にする
            std::string format;
            for (unsigned char c : texture->achFormatHint) {
                if (c == '\0') break;  // null文字は入れない
                format += static_cast<char>(std::tolower(c));
            }
            imageAsset.format = format;
        }
        outModel.images.push_back(std::move(imageAsset));
    }

    // メッシュの読み込み
    outModel.meshes.reserve(scene->mNumMeshes);  // メモリ確保

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        MeshAsset meshAsset;

        // 頂点データの読み込み
        if (!ParseMesh(mesh, meshAsset)) {
            continue;
        }
        outModel.meshes.push_back(std::move(meshAsset));
    }

    // マテリアルの読み込み
    outModel.materials.reserve(scene->mNumMaterials);  // メモリ確保

    for (int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* material = scene->mMaterials[i];
        MaterialAsset materialAsset;

        // マテリアルデータの読み込み
        int imageCount = static_cast<int>(outModel.images.size());
        if (!ParseMaterial(material, imageCount, materialAsset)) {
            continue;
        }
        outModel.materials.push_back(std::move(materialAsset));
    }

    return true;
}

bool GLBImporter::ParseMesh(const aiMesh* srcMesh, MeshAsset& outMesh) {
    if (!srcMesh) {
        return false;
    }

    // 頂点データの読み込み
    outMesh.vertices.resize(srcMesh->mNumVertices);  // 頂点数分のメモリ確保

    for (auto i = 0; i < srcMesh->mNumVertices; i++) {
        StandardVertex& vertex = outMesh.vertices[i];

        // 座標
        vertex.position.x = srcMesh->mVertices[i].x;
        vertex.position.y = srcMesh->mVertices[i].y;
        vertex.position.z = srcMesh->mVertices[i].z;

        // 法線
        if (srcMesh->HasNormals()) {
            vertex.normal.x = srcMesh->mNormals[i].x;
            vertex.normal.y = srcMesh->mNormals[i].y;
            vertex.normal.z = srcMesh->mNormals[i].z;
        }

        // 接線ベクトル
        if (srcMesh->HasTangentsAndBitangents()) {
            vertex.tangent.x = srcMesh->mTangents[i].x;
            vertex.tangent.y = srcMesh->mTangents[i].y;
            vertex.tangent.z = srcMesh->mTangents[i].z;
        }

        // UV座標
        if (srcMesh->HasTextureCoords(0)) {
            vertex.texcoord.x = srcMesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = srcMesh->mTextureCoords[0][i].y;
        }

        // 頂点カラー
        if (srcMesh->HasVertexColors(0)) {
            vertex.color.x = srcMesh->mColors[0][i].r;
            vertex.color.y = srcMesh->mColors[0][i].g;
            vertex.color.z = srcMesh->mColors[0][i].b;
            vertex.color.w = srcMesh->mColors[0][i].a;
        }
    }

    // インデックスデータの読み込み
    outMesh.indices.resize(srcMesh->mNumFaces * 3);
    for (unsigned int i = 0; i < srcMesh->mNumFaces; i++) {
        const aiFace& face = srcMesh->mFaces[i];
        // 三角形にしているので頂点は3つ
        outMesh.indices[i * 3 + 0] = face.mIndices[0];
        outMesh.indices[i * 3 + 1] = face.mIndices[1];
        outMesh.indices[i * 3 + 2] = face.mIndices[2];
    }

    // マテリアルIDの設定
    outMesh.materialID = srcMesh->mMaterialIndex;

    return true;
}

bool GLBImporter::ParseMaterial(
    const aiMaterial* srcMaterial, int imageCount, MaterialAsset& outMaterial) {
    // 引数チェック
    if (!srcMaterial) {
        return false;
    }

    // マテリアル名
    aiString name;
    if (srcMaterial->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
        outMaterial.name =
            std::wstring(name.C_Str(), name.C_Str() + strlen(name.C_Str()));
    }

    // baseColor
    aiColor4D baseColor;
    if (srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS) {
        outMaterial.baseColorFactor = DirectX::XMFLOAT4(
            baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }

    // metallic
    float metallic = 0.0f;
    if (srcMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        outMaterial.metallicFactor = metallic;
    }

    // roughness
    float roughness = 1.0f;
    if (srcMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        outMaterial.roughnessFactor = roughness;
    }

    // emissive
    aiColor3D emissive;
    if (srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
        outMaterial.emissiveFactor =
            DirectX::XMFLOAT3(emissive.r, emissive.g, emissive.b);
    }

    // occlusion factorはassimpでは取得できない

    // テクスチャ参照の設定
    // base color
    aiString baseColorPath;
    if (srcMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColorPath) ==
        AI_SUCCESS) {
        std::string pathStr = baseColorPath.C_Str();
        // assimpでは先頭が*なら埋め込みテクスチャを示すため，
        // glbの画像では，*0, *1, ...のようになる
        if (pathStr[0] == '*') {
            // 数字部分を抜き出してインデックスとして記録する
            int texIndex = std::atoi(pathStr.substr(1).c_str());
            if (texIndex >= 0 && texIndex < imageCount) {
                outMaterial.baseColorLocalTextureIndex = texIndex;
            }
        }
    }

    // metallic-roughness
    aiString metallicRoughnessPath;
    if (srcMaterial->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0,
            &metallicRoughnessPath) == AI_SUCCESS) {
        std::string pathStr = metallicRoughnessPath.C_Str();
        if (pathStr[0] == '*') {  // 埋め込みテクスチャ
            int texIndex = std::atoi(pathStr.substr(1).c_str());
            if (texIndex >= 0 && texIndex < imageCount) {
                outMaterial.metallicRoughnessLocalTextureIndex = texIndex;
            }
        }
    }

    // occlusion
    aiString occlusionPath;
    if (srcMaterial->GetTexture(
            aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath) == AI_SUCCESS) {
        std::string pathStr = occlusionPath.C_Str();
        if (pathStr[0] == '*') {
            int texIndex = std::atoi(pathStr.substr(1).c_str());
            if (texIndex >= 0 && texIndex < imageCount) {
                outMaterial.occlusionLocalTextureIndex = texIndex;
            }
        }
    }

    // normal
    aiString normalPath;
    if (srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalPath) ==
        AI_SUCCESS) {
        std::string pathStr = normalPath.C_Str();
        if (pathStr[0] == '*') {
            int texIndex = std::atoi(pathStr.substr(1).c_str());
            if (texIndex >= 0 && texIndex < imageCount) {
                outMaterial.normalLocalTextureIndex = texIndex;
            }
        }
    }

    // emissive
    aiString emissivePath;
    if (srcMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissivePath) ==
        AI_SUCCESS) {
        std::string pathStr = emissivePath.C_Str();
        if (pathStr[0] == '*') {
            int texIndex = std::atoi(pathStr.substr(1).c_str());
            if (texIndex >= 0 && texIndex < imageCount) {
                outMaterial.emissiveLocalTextureIndex = texIndex;
            }
        }
    }

    return true;
}