#include "Engine/ModelLoader.h"

#include "Engine/AssetPath.h"
#include "Engine/GLBImporter.h"
#include "Engine/ModelAsset.h"

bool ModelLoader::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    TextureManager* pTextureManager) {
    // 引数チェック
    if (!pDevice || !pPoolCBV || !pTextureManager) {
        return false;
    }

    m_pDevice         = pDevice;
    m_pPoolCBV        = pPoolCBV;
    m_pTextureManager = pTextureManager;

    return true;
}

void ModelLoader::Term() {
    m_pDevice         = nullptr;
    m_pPoolCBV        = nullptr;
    m_pTextureManager = nullptr;
}

std::unique_ptr<Model> ModelLoader::LoadModel(
    const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch) {
    // GLBの読み込み
    ModelAsset modelAsset;
    if (!GLBImporter::LoadFromFile(path, modelAsset)) {
        return nullptr;
    }
    uint32_t m_textureCount = static_cast<UINT>(modelAsset.images.size());

    // テクスチャ生成
    m_pTextureManager->BuildTexturesFromModelAsset(modelAsset, batch);

    // モデルのGPUリソース生成
    auto model = std::make_unique<Model>();
    if (!model->Init(
            m_pDevice, m_pPoolCBV, m_pTextureManager, batch, modelAsset)) {
        return nullptr;
    }

    return model;
}
