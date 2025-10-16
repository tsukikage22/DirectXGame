#include "Engine/FBXImporter.h"

bool FBXImporter::LoadFromFile(const std::filesystem::path& path,
    std::vector<MeshAsset>& outMeshes,
    std::vector<MaterialAsset>& outMaterials) {
    // ファイルパスの確認
    if (!std::filesystem::exists(path)) {
        return false;
    }

    // 初期化
    outMeshes.clear();
    outMaterials.clear();

    static Assimp::Importer importer;
    unsigned int flags = 0;
    flags |= aiProcess_Triangulate |               // 三角形化
             aiProcess_PreTransformVertices |      // 変換の適用
             aiProcess_GenSmoothNormals |          // スムース法線ベクトル生成
             aiProcess_CalcTangentSpace |          // 接線ベクトル計算
             aiProcess_GenUVCoords |               // UV座標生成
             aiProcess_RemoveRedundantMaterials |  // 冗長なマテリアルの削除
             aiProcess_OptimizeMeshes;             // メッシュの最適化

    const aiScene* scene = importer.ReadFile(path.string(), flags);

    // エラーチェック
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        printf(importer.GetErrorString());
        return false;
    }

    // メッシュデータの読み込み
    outMeshes.reserve(scene->mNumMeshes);  // メッシュ数分のメモリ確保

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        MeshAsset meshAsset;

        // 頂点データの読み込み
        if (ParseMesh(mesh, meshAsset)) {
            continue;
        }
        outMeshes.push_back(meshAsset);
    }

    // マテリアルデータの読み込み
    outMaterials.reserve(scene->mNumMaterials);  // マテリアル数分のメモリ確保

    for (int i = 0; i < outMaterials.size(); i++) {
        const aiMaterial* material = scene->mMaterials[i];
        MaterialAsset materialAsset;

        // マテリアルデータの読み込み
        if (ParseMaterial(material, materialAsset)) {
            continue;
        }
        outMaterials.push_back(materialAsset);
    }

    return true;
}

bool FBXImporter::ParseMesh(const aiMesh* srcMesh, MeshAsset& outMesh) {
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

        // UV座標
        if (srcMesh->HasTextureCoords(0)) {
            vertex.uv.x = srcMesh->mTextureCoords[0][i].x;
            vertex.uv.y = srcMesh->mTextureCoords[0][i].y;
        }

        // 接線ベクトル
        if (srcMesh->HasTangentsAndBitangents()) {
            vertex.tangent.x = srcMesh->mTangents[i].x;
            vertex.tangent.y = srcMesh->mTangents[i].y;
            vertex.tangent.z = srcMesh->mTangents[i].z;
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

bool FBXImporter::ParseMaterial(
    const aiMaterial* material, MaterialAsset& outMaterial) {
    // 引数チェック
    if (!material) {
        return false;
    }

    // マテリアル名
    aiString name;
    if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
        outMaterial.name =
            std::wstring(name.C_Str(), name.C_Str() + strlen(name.C_Str()));
    }

    // baseColor
    aiColor4D baseColor;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS) {
        outMaterial.baseColor = DirectX::XMFLOAT4(
            baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    } else {
        outMaterial.baseColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
    }

    // metallic
    float metallic = 0.0f;
    if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        outMaterial.metallic = metallic;
    }

    // roughness
    float roughness = 1.0f;
    if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        outMaterial.roughness = roughness;
    }

    // ベースカラーのテクスチャパス
    aiString baseColorPath;
    if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColorPath) ==
        AI_SUCCESS) {
        outMaterial.baseColorPath = std::wstring(baseColorPath.C_Str(),
            baseColorPath.C_Str() + strlen(baseColorPath.C_Str()));
    }

    // 法線マップのテクスチャパス
    aiString normalPath;
    if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath) ==
        AI_SUCCESS) {
        outMaterial.normalPath = std::wstring(normalPath.C_Str(),
            normalPath.C_Str() + strlen(normalPath.C_Str()));
    }

    aiString metallicPath;
    if (material->GetTexture(aiTextureType_METALNESS, 0, &metallicPath) ==
        AI_SUCCESS) {
        outMaterial.metallicPath = std::wstring(metallicPath.C_Str(),
            metallicPath.C_Str() + strlen(metallicPath.C_Str()));
    }

    aiString roughnessPath;
    if (material->GetTexture(
            aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath) == AI_SUCCESS) {
        outMaterial.roughnessPath = std::wstring(roughnessPath.C_Str(),
            roughnessPath.C_Str() + strlen(roughnessPath.C_Str()));
    }

    return true;
}