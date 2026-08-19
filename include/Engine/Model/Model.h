/// @file Model.h
/// @brief MeshとMaterialのリソース管理
#pragma once

#include <memory>
#include <vector>

#include "Engine/Model/MaterialGPU.h"
#include "Engine/Model/MeshGPU.h"

class GraphicsDevice;

class Model {
public:
    Model() = default;
    ~Model() { Term(); };

    /// @brief 初期化，ModelAssetからGPUリソースを作成
    bool Init(GraphicsDevice& graphicsDevice, TextureManager* pTextureManager,
        DirectX::ResourceUploadBatch& batch, const ModelAsset& modelAsset);

    /// @brief リソースの破棄
    void Term();

    /// @brief アップロードヒープの破棄
    void DiscardUpload();

    //========================================
    // アクセサ
    //========================================
    const std::vector<std::unique_ptr<MeshGPU>>& GetMeshes() const {
        return m_meshes;
    }
    const std::vector<std::unique_ptr<MaterialGPU>>& GetMaterials() const {
        return m_materials;
    }

private:
    std::vector<std::unique_ptr<MeshGPU>> m_meshes;         // メッシュ
    std::vector<std::unique_ptr<MaterialGPU>> m_materials;  // マテリアル
};