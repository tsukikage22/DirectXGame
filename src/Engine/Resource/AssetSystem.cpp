#include "Engine/Resource/AssetSystem.h"

#include <directxtk12/ResourceUploadBatch.h>

#include <memory>

#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Resource/AssetLoadScope.h"
#include "Engine/Resource/EnvironmentMap.h"

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

    // IBLBakerの初期化
    if (!m_iblBaker.Init(&graphicsDevice)) {
        OutputDebugStringW(L"Failed to initialize IBLBaker.\n");
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

    // EnvironmentMapの初期化（デフォルトキューブマップの作成を含むのでbatchが必要）
    if (!m_environmentMap.Init(&graphicsDevice, batch)) {
        OutputDebugStringW(L"Failed to initialize EnvironmentMap.\n");
        return false;
    }

    // BRDF LUT構築
    if (!m_iblBaker.BakeBrdfLut(m_environmentMap)) {
        OutputDebugStringW(L"Failed to bake BRDF LUT.\n");
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

    // IBLBakerの終了処理
    m_iblBaker.Term();

    // EnvironmentMapの終了処理
    m_environmentMap.Term();

    // IESProfileの終了処理
    m_iesProfile.Term();

    // ModelLoaderの終了処理
    m_modelLoader.Term();
}

bool AssetSystem::BuildEnvironmentMap(const std::filesystem::path& path) {
    if (m_pGraphicsDevice == nullptr) {
        OutputDebugStringW(L"GraphicsDevice is not initialized.\n");
        return false;
    }

    // ResourceUploadBatchの生成
    DirectX::ResourceUploadBatch batch(m_pGraphicsDevice->GetDevice());
    batch.Begin();

    // HDRI読み込み
    if (!m_environmentMap.LoadHDRI(path, batch)) {
        OutputDebugStringW(L"Failed to load HDRI for EnvironmentMap.\n");
        auto future =
            batch.End(m_pGraphicsDevice->GetCommandQueue().GetD3DQueue());
        future.wait();
        return false;
    }
    // アップロード待機
    auto future = batch.End(m_pGraphicsDevice->GetCommandQueue().GetD3DQueue());
    future.wait();

    // 環境キューブマップ構築
    if (!m_iblBaker.EquirectToCubemap(m_environmentMap)) {
        OutputDebugStringW(L"Failed to build cubemap for EnvironmentMap.\n");
        return false;
    }

    // 環境キューブマップのミップマップ生成
    if (!m_iblBaker.GenerateEnvCubemapMips(m_environmentMap)) {
        OutputDebugStringW(
            L"Failed to generate mipmaps for EnvironmentMap cubemap.\n");
        return false;
    }

    // 照度マップ構築
    if (!m_iblBaker.BakeIrradianceMap(m_environmentMap)) {
        OutputDebugStringW(L"Failed to bake irradiance map.\n");
        return false;
    }

    // prefiltered env map構築
    if (!m_iblBaker.BakePrefilteredEnvMap(m_environmentMap)) {
        OutputDebugStringW(L"Failed to bake prefiltered env map.\n");
        return false;
    }

    return true;
}

// AssetLoadScopeの作成
AssetLoadScope AssetSystem::CreateAssetLoadScope(Scene& scene) {
    // batchのBegin
    auto batch = std::make_unique<DirectX::ResourceUploadBatch>(
        m_pGraphicsDevice->GetDevice());
    batch->Begin();

    return AssetLoadScope(std::move(batch),
        m_pGraphicsDevice->GetCommandQueue(), m_modelLoader, scene,
        m_iesProfile);
}