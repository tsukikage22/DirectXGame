/// @file Model.h
/// @brief MeshとMaterialのリソース管理
#pragma once

#include <d3d12.h>

#include <memory>
#include <vector>

#include "Engine/MaterialGPU.h"
#include "Engine/MeshGPU.h"

class Model {
public:
    Model()  = default;
    ~Model() = default;

    /// @brief 初期化，ModelAssetからGPUリソースを作成
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV_SRV_UAV,
        TextureManager* pTextureManager, DirectX::ResourceUploadBatch& batch,
        const ModelAsset& modelAsset);

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