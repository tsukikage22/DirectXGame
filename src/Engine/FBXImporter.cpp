#include "Engine/FBXImporter.h"

bool FBXImporter::LoadFromFile(
    const std::filesystem::path& path, std::vector<MeshAsset>& outMeshes) {
    // ファイルパスの確認
    if (!std::filesystem::exists(path)) {
        return false;
    }

    // 初期化
    outMeshes.clear();

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
        return nullptr;
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