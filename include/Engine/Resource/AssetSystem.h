/// @file AssetSystem.h
/// @brief ModelLoaderやTextureManagerの初期化など

#pragma once

#include "Engine/Resource/EnvironmentMap.h"
#include "Engine/Resource/IESProfile.h"
#include "Engine/Resource/ModelLoader.h"
#include "Engine/Resource/TextureManager.h"

// 前方宣言
class GraphicsDevice;
class AssetLoadScope;
class Scene;

class AssetSystem {
public:
    AssetSystem()  = default;
    ~AssetSystem() = default;

    /// @brief 初期化
    bool Init(GraphicsDevice& graphicsDevice);

    /// @brief 終了処理
    void Term();

    /// @brief AssetLoadScopeの作成
    /// @param queue コマンドキュー
    /// @param scene シーン
    /// @return AssetLoadScope
    AssetLoadScope CreateAssetLoadScope(Scene& scene);

    /// @brief IESプロファイルのSRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetIesSrvGpuHandle() const {
        return m_iesProfile.GetSrvGpuHandle();
    }

    /// @brief EnvironmentMapのSRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvironmentMapSrvGpuHandle() const {
        return m_environmentMap.GetSrvGpuHandle();
    }

private:
    TextureManager m_textureManager;
    ModelLoader m_modelLoader;
    IESProfile m_iesProfile;
    EnvironmentMap m_environmentMap;

    GraphicsDevice* m_pGraphicsDevice = nullptr;
};
