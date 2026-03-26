#include "Engine/Model.h"

bool Model::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV_SRV_UAV,
    TextureManager* pTextureManager, DirectX::ResourceUploadBatch& batch,
    const ModelAsset& modelAsset) {
    // 引数チェック
    if (!pDevice || !pPoolCBV_SRV_UAV || !pTextureManager ||
        !modelAsset.IsValid()) {
        return false;
    }

    // メッシュをGPUに転送
    m_meshes.resize(modelAsset.meshes.size());
    for (size_t i = 0; i < modelAsset.meshes.size(); i++) {
        m_meshes[i] = std::make_unique<MeshGPU>();
        if (!m_meshes[i]->Init(pDevice, batch, modelAsset.meshes[i])) {
            return false;
        }
    }

    // マテリアルをGPUに転送
    m_materials.resize(modelAsset.materials.size());
    for (size_t i = 0; i < modelAsset.materials.size(); i++) {
        m_materials[i] = std::make_unique<MaterialGPU>();
        if (!m_materials[i]->Init(pDevice,
                pPoolCBV_SRV_UAV,  // CBV用（定数バッファ）
                pPoolCBV_SRV_UAV,  // SRV用（テクスチャコピー先）
                pTextureManager,   // テクスチャソース
                modelAsset.materials[i])) {
            return false;
        }
    }

    return true;
}

void Model::Term() {
    // メッシュの破棄
    for (auto& mesh : m_meshes) {
        mesh->Term();
    }
    m_meshes.clear();

    // マテリアルの破棄
    for (auto& material : m_materials) {
        material->Term();
    }
    m_materials.clear();
}

void Model::DiscardUpload() {
    for (auto& mesh : m_meshes) {
        mesh->DiscardUpload();
    }
}