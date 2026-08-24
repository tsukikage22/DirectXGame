/// @file EnvironmentMap.h
/// @brief 環境マップ

#pragma once

#include <directxtk12/ResourceUploadBatch.h>

#include <filesystem>

#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Core/DescriptorPool.h"
#include "Engine/Resource/TextureResource.h"

class GraphicsDevice;

class EnvironmentMap {
public:
    EnvironmentMap()  = default;
    ~EnvironmentMap() = default;

    /// @brief 初期化処理，
    /// @param pDevice デバイス
    /// @param pPoolSRV SRV用ディスクリプタプール
    /// @return 初期化に成功したかどうか
    bool Init(GraphicsDevice* pDevice, DescriptorPool* pPoolSRV);

    /// @brief 終了処理，リソースの破棄
    void Term();

    /// @brief リソースの作成とHDRIの読み込み
    /// @param filePath HDRファイルのパス
    /// @param batch リソースアップロードバッチ
    /// @return 読み込みに成功したかどうか
    bool LoadHDRI(const std::filesystem::path& filePath,
        DirectX::ResourceUploadBatch& batch);

    /// @brief SRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const;

private:
    GraphicsDevice* m_pDevice = nullptr;   // デバイス
    TextureResource m_resource;            // 環境マップテクスチャのリソース
    DescriptorPool* m_pPoolSRV = nullptr;  // SRV用ディスクリプタプール
    DescriptorAllocation m_srv;            // SRVディスクリプタ
};
