#include "Engine/Model/Model.h"

#include "Engine/Core/GraphicsDevice.h"

bool Model::Init(GraphicsDevice& graphicsDevice,
    TextureManager* pTextureManager, DirectX::ResourceUploadBatch& batch,
    const ModelAsset& modelAsset) {
    // 引数チェック
    if (!pTextureManager || !modelAsset.IsValid()) {
        return false;
    }

    ID3D12Device* pDevice = graphicsDevice.GetDevice();

    // メッシュをGPUに転送
    m_meshes.reserve(modelAsset.meshes.size());
    for (size_t i = 0; i < modelAsset.meshes.size(); i++) {
        auto mesh = std::make_unique<MeshGPU>();
        if (!mesh->Init(pDevice, batch, modelAsset.meshes[i])) {
            Term();
            return false;
        }
        m_meshes.push_back(std::move(mesh));
    }

    // マテリアルをGPUに転送
    m_materials.reserve(modelAsset.materials.size());
    for (size_t i = 0; i < modelAsset.materials.size(); i++) {
        auto material = std::make_unique<MaterialGPU>();
        if (!material->Init(
                graphicsDevice, pTextureManager, modelAsset.materials[i])) {
            Term();
            return false;
        }
        m_materials.push_back(std::move(material));
    }

    return true;
}

void Model::Term() {
    // メッシュの破棄
    m_meshes.clear();

    // マテリアルの破棄
    m_materials.clear();
}

void Model::DiscardUpload() {
    for (auto& mesh : m_meshes) {
        mesh->DiscardUpload();
    }
}