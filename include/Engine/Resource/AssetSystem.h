/// @file AssetSystem.h
/// @brief ModelLoaderやTextureManagerの初期化など

#pragma once

#include "Engine/Render/IBLBaker.h"
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

    /// @brief 環境マップの読み込みとキューブマップの作成
    /// @param path HDRIファイルのパス
    /// @return 読み込みに成功したかどうか
    [[nodiscard]] bool BuildEnvironmentMap(const std::filesystem::path& path);

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
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapEquirectSrvGpuHandle() const {
        return m_environmentMap.GetEquirectSrvGpuHandle();
    }

    /// @brief EnvironmentMapのキューブマップSRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapCubemapSrvGpuHandle() const {
        return m_environmentMap.GetCubemapSrvGpuHandle();
    }

    /// @brief EnvironmentMapのキューブマップUAVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapCubemapUavGpuHandle() const {
        return m_environmentMap.GetCubemapUavGpuHandle();
    }

    /// @brief EnvironmentMapのirradiance map SRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapIrradianceSrvGpuHandle() const {
        return m_environmentMap.GetIrradianceSrvGpuHandle();
    }

    /// @brief EnvironmentMapのprefiltered map SRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapPrefilteredSrvGpuHandle() const {
        return m_environmentMap.GetPrefilteredSrvGpuHandle();
    }

    /// @brief EnvironmentMapのBRDF LUT SRVハンドルを取得する
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvMapBrdfLutSrvGpuHandle() const {
        return m_environmentMap.GetBrdfLutSrvGpuHandle();
    }

private:
    TextureManager m_textureManager;
    ModelLoader m_modelLoader;
    IESProfile m_iesProfile;
    EnvironmentMap m_environmentMap;
    IBLBaker m_iblBaker;

    GraphicsDevice* m_pGraphicsDevice = nullptr;
};
