#include "Engine/Resource/AssetSystem.h"

#include <directxtk12/ResourceUploadBatch.h>

#include <memory>

#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Resource/AssetLoadScope.h"

bool AssetSystem::Init(GraphicsDevice& graphicsDevice) {
    m_pGraphicsDevice = &graphicsDevice;

    // TextureManagerの初期化
    if (!m_textureManager.Init(graphicsDevice.GetDevice())) {
        OutputDebugStringW(L"Failed to initialize TextureManager.\n");
        return false;
    }

    // ModelLoaderの初期化
    if (!m_modelLoader.Init(graphicsDevice, m_textureManager)) {
        OutputDebugStringW(L"Failed to initialize ModelLoader.\n");
        return false;
    }

    // IESProfileの初期化
    if (!m_iesProfile.Init(graphicsDevice)) {
        OutputDebugStringW(L"Failed to initialize IESProfile.\n");
        return false;
    }

    // EnvironmentMapの初期化
    if (!m_environmentMap.Init(
            &graphicsDevice, graphicsDevice.CbvSrvUavPool())) {
        OutputDebugStringW(L"Failed to initialize EnvironmentMap.\n");
        return false;
    }

    // ResourceUploadBatchの生成
    DirectX::ResourceUploadBatch batch(graphicsDevice.GetDevice());
    batch.Begin();

    // デフォルトテクスチャの生成
    if (!m_textureManager.CreateDefaultTextures(batch)) {
        OutputDebugStringW(L"Failed to create default textures.\n");
        return false;
    }

    // アップロード待機
    auto future = batch.End(graphicsDevice.GetCommandQueue().GetD3DQueue());
    future.wait();

    return true;
}

void AssetSystem::Term() {
    // TextureManagerの終了処理
    m_textureManager.Term();

    // EnvironmentMapの終了処理
    m_environmentMap.Term();

    // IESProfileの終了処理
    m_iesProfile.Term();

    // ModelLoaderの終了処理
    m_modelLoader.Term();
}

// AssetLoadScopeの作成
AssetLoadScope AssetSystem::CreateAssetLoadScope(Scene& scene) {
    // batchのBegin
    auto batch = std::make_unique<DirectX::ResourceUploadBatch>(
        m_pGraphicsDevice->GetDevice());
    batch->Begin();

    return AssetLoadScope(std::move(batch),
        m_pGraphicsDevice->GetCommandQueue(), m_modelLoader, scene,
        m_iesProfile, m_environmentMap);
}