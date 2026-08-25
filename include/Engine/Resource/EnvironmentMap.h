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

    /// @brief  キューブマップのサイズを取得する
    uint32_t GetCubemapSize() const { return kCubeMapSize; }

    /// @brief equirect SRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetEquirectSrvGpuHandle() const;

    /// @brief キューブマップUAVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetCubemapUavGpuHandle() const;

    /// @brief キューブマップSRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetCubemapSrvGpuHandle() const;

    /// @brief equirectリソースの取得
    ID3D12Resource* GetEquirectResource() const {
        return m_equirectMap.GetResource();
    }

    /// @brief  キューブマップリソースの取得
    ID3D12Resource* GetCubemapResource() const {
        return m_cubeMap.GetResource();
    }

private:
    constexpr static uint32_t kCubeMapSize = 1024;  // キューブマップのサイズ

    GraphicsDevice* m_pDevice  = nullptr;  // デバイス
    DescriptorPool* m_pPoolSRV = nullptr;  // SRV用ディスクリプタプール
    DescriptorAllocation m_equirectSrv;    // SRVディスクリプタ
    DescriptorAllocation m_cubemapUav;     // キューブマップUAVディスクリプタ
    DescriptorAllocation m_cubemapSrv;     // キューブマップSRVディスクリプタ

    TextureResource m_equirectMap;  // 環境マップテクスチャのリソース
    TextureResource m_cubeMap;      // キューブマップテクスチャのリソース
};
