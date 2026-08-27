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
    /// @param batch リソースアップロードバッチ
    /// @return 初期化に成功したかどうか
    bool Init(GraphicsDevice* pDevice, DirectX::ResourceUploadBatch& batch);

    /// @brief 終了処理，リソースの破棄
    void Term();

    /// @brief リソースの作成とHDRIの読み込み
    /// @param filePath HDRファイルのパス
    /// @param batch リソースアップロードバッチ
    /// @return 読み込みに成功したかどうか
    bool LoadHDRI(const std::filesystem::path& filePath,
        DirectX::ResourceUploadBatch& batch);

    /// @brief equirect SRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetEquirectSrvGpuHandle() const;

    /// @brief 環境キューブマップUAVディスクリプタの取得
    /// @param mip ミップレベル（0～kEnvCubeMipLevels-1）
    D3D12_GPU_DESCRIPTOR_HANDLE GetCubemapUavGpuHandle(uint32_t mip = 0) const;

    /// @brief 環境キューブマップSRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetCubemapSrvGpuHandle() const;

    /// @brief 環境キューブマップミップSRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetCubemapMipSrvGpuHandle(uint32_t mip) const;

    /// @brief 照度マップUAVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetIrradianceUavGpuHandle() const;

    /// @brief 照度マップSRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetIrradianceSrvGpuHandle() const;

    /// @brief prefiltered map UAVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetPrefilteredUavGpuHandle(uint32_t mip) const;

    /// @brief prefiltered map SRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetPrefilteredSrvGpuHandle() const;

    /// @brief BRDF LUT UAVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetBrdfLutUavGpuHandle() const;

    /// @brief BRDF LUT SRVディスクリプタの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetBrdfLutSrvGpuHandle() const;

    /// @brief equirectリソースの取得
    ID3D12Resource* GetEquirectResource() const {
        return m_equirectMap.GetResource();
    }

    /// @brief  キューブマップリソースの取得
    ID3D12Resource* GetCubemapResource() const {
        return m_cubeMap.GetResource();
    }

    /// @brief  照度マップリソースの取得
    ID3D12Resource* GetIrradianceResource() const {
        return m_irradianceMap.GetResource();
    }

    /// @brief  prefiltered mapリソースの取得
    ID3D12Resource* GetPrefilteredResource() const {
        return m_prefilteredMap.GetResource();
    }

    /// @brief  BRDF LUTリソースの取得
    ID3D12Resource* GetBrdfLutResource() const {
        return m_BrdfLut.GetResource();
    }

    //======================================================================
    // constants
    //======================================================================
    static constexpr uint32_t kCubeMapSize = 1024;  // キューブマップのサイズ
    static constexpr uint32_t kEnvCubeMipLevels  = 11;  // 環境マップのミップ
    static constexpr uint32_t kIrradianceMapSize = 32;  // 照度マップのサイズ
    static constexpr uint32_t kPrefilteredSize =
        128;  // prefiltered mapのサイズ
    static constexpr uint32_t kPrefilteredMipLevels =
        5;                                         // prefiltered mapのミップ
    static constexpr uint32_t kBrdfLutSize = 512;  // BRDF LUTのサイズ
    static constexpr DXGI_FORMAT kPrefilteredFormat =
        DXGI_FORMAT_R16G16B16A16_FLOAT;  // prefiltered mapのフォーマット
    static constexpr DXGI_FORMAT kCubemapFormat =
        DXGI_FORMAT_R16G16B16A16_FLOAT;  // キューブマップのフォーマット
    static constexpr DXGI_FORMAT kIrradianceMapFormat =
        DXGI_FORMAT_R32G32B32A32_FLOAT;  // 照度マップのフォーマット
    static constexpr DXGI_FORMAT kBrdfLutFormat =
        DXGI_FORMAT_R16G16_FLOAT;  // BRDF LUTのフォーマット

private:
    GraphicsDevice* m_pDevice     = nullptr;  // デバイス
    DescriptorPool* m_pPoolSrvUav = nullptr;  // SRV用ディスクリプタプール
    DescriptorAllocation m_equirectSrv;       // HDRIのSRV
    DescriptorAllocation m_cubemapUav;        // キューブマップUAV
    DescriptorAllocation m_cubemapSrv;        // キューブマップSRV
    DescriptorAllocation m_cubemapMipSrv;     // キューブマップミップSRV
    DescriptorAllocation m_irradianceUav;     // 照度マップUAV
    DescriptorAllocation m_irradianceSrv;     // 照度マップSRV
    DescriptorAllocation m_prefilteredUav;    // prefiltered map UAV
    DescriptorAllocation m_prefilteredSrv;    // prefiltered map SRV
    DescriptorAllocation m_BrdfLutSrv;        // BRDF LUT SRV
    DescriptorAllocation m_BrdfLutUav;        // BRDF LUT UAV
    DescriptorAllocation m_defaultSrv;        // デフォルトキューブマップSRV

    TextureResource m_equirectMap;     // 環境マップのリソース
    TextureResource m_cubeMap;         // 環境キューブマップのリソース
    TextureResource m_irradianceMap;   // 照度マップのリソース
    TextureResource m_prefilteredMap;  // prefiltered mapのリソース
    TextureResource m_BrdfLut;         // BRDF LUTのリソース
    TextureResource
        m_defaultCubeMap;  // デフォルトキューブマップテクスチャのリソース

    bool m_canUseCubemap = false;  // キューブマップが使用可能かどうか
};
