#include "Engine/Resource/ModelLoader.h"

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Model/ModelAsset.h"
#include "Engine/Resource/AssetPath.h"
#include "Engine/Resource/GLBImporter.h"
#include "Engine/Resource/TextureManager.h"

bool ModelLoader::Init(
    GraphicsDevice& graphicsDevice, TextureManager* pTextureManager) {
    // 引数チェック
    if (!pTextureManager) {
        return false;
    }

    m_pGraphicsDevice = &graphicsDevice;
    m_pTextureManager = pTextureManager;

    return true;
}

void ModelLoader::Term() {
    m_pGraphicsDevice = nullptr;
    m_pTextureManager = nullptr;
}

std::unique_ptr<Model> ModelLoader::LoadModel(
    const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch) {
    // 初期化チェック
    if (!m_pGraphicsDevice || !m_pTextureManager) {
        return nullptr;
    }

    // GLBの読み込み
    ModelAsset modelAsset;
    if (!GLBImporter::LoadFromFile(path, modelAsset)) {
        return nullptr;
    }

    // テクスチャ生成
    m_pTextureManager->BuildTexturesFromModelAsset(modelAsset, batch);

    // モデルのGPUリソース生成
    auto model = std::make_unique<Model>();
    if (!model->Init(
            *m_pGraphicsDevice, m_pTextureManager, batch, modelAsset)) {
        return nullptr;
    }

    return model;
}
