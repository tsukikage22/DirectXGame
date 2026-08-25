#include "Engine/Resource/EnvironmentMap.h"

#include <DirectXTex.h>

#include <vector>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"

bool EnvironmentMap::Init(GraphicsDevice* pDevice, DescriptorPool* pPoolSRV) {
    Term();
    if (!pDevice || !pPoolSRV) {
        return false;
    }

    m_pDevice  = pDevice;
    m_pPoolSRV = pPoolSRV;

    // キューブマップ用リソースの作成
    if (!m_cubeMap.InitAsTexture2DArray(m_pDevice->GetDevice(), kCubeMapSize,
            kCubeMapSize, DXGI_FORMAT_R16G16B16A16_FLOAT, 6, 1,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
        return false;
    }

    // UAVの作成
    m_cubemapUav                             = m_pPoolSRV->Allocate();
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.MipSlice        = 0;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize       = 6;
    uavDesc.Format                         = DXGI_FORMAT_R16G16B16A16_FLOAT;
    m_pDevice->GetDevice()->CreateUnorderedAccessView(m_cubeMap.GetResource(),
        nullptr, &uavDesc, m_cubemapUav.GetCPUHandle());

    // SRVの作成
    m_cubemapSrv                            = m_pPoolSRV->Allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip     = 0;
    srvDesc.TextureCube.MipLevels           = 1;
    srvDesc.Format                          = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_pDevice->GetDevice()->CreateShaderResourceView(
        m_cubeMap.GetResource(), &srvDesc, m_cubemapSrv.GetCPUHandle());

    return true;
}

void EnvironmentMap::Term() {
    m_equirectMap.Term();
    m_cubeMap.Term();
    m_equirectSrv = {};
    m_cubemapUav  = {};
    m_cubemapSrv  = {};
    m_pPoolSRV    = nullptr;
    m_pDevice     = nullptr;
}

bool EnvironmentMap::LoadHDRI(const std::filesystem::path& filePath,
    DirectX::ResourceUploadBatch& batch) {
    if (!m_pDevice || !m_pPoolSRV || filePath.empty()) {
        return false;
    }

    // HDRファイルの読み込み
    DirectX::ScratchImage srcImage;
    CHECK_HR(m_pDevice->GetDevice(),
        DirectX::LoadFromHDRFile(filePath.c_str(), nullptr, srcImage));

    // halfの表現範囲（65504）を超える値がINFになることを防ぐため
    // Convert前にクランプする
    DirectX::ScratchImage clampedImage;
    CHECK_HR(m_pDevice->GetDevice(),
        DirectX::TransformImage(
            srcImage.GetImages(), srcImage.GetImageCount(),
            srcImage.GetMetadata(),
            [](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels,
                size_t width, [[maybe_unused]] size_t y) {
                static const DirectX::XMVECTORF32 s_max = {
                    { { 65504.0f, 65504.0f, 65504.0f, 65504.0f } }
                };
                for (size_t j = 0; j < width; ++j) {
                    outPixels[j] = DirectX::XMVectorClamp(
                        inPixels[j], DirectX::g_XMZero, s_max);
                }
            },
            clampedImage));
    srcImage.Release();

    // R32G32B32A32_FLOATからR16G16B16A16_FLOATに変換
    DirectX::ScratchImage convertedImage;
    CHECK_HR(m_pDevice->GetDevice(),
        DirectX::Convert(clampedImage.GetImages(), clampedImage.GetImageCount(),
            clampedImage.GetMetadata(), DXGI_FORMAT_R16G16B16A16_FLOAT,
            DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT,
            convertedImage));

    // 環境マップテクスチャのリソース作成
    UINT width  = static_cast<UINT>(convertedImage.GetMetadata().width);
    UINT height = static_cast<UINT>(convertedImage.GetMetadata().height);
    if (!m_equirectMap.InitAsTexture2D(m_pDevice->GetDevice(), width, height,
            convertedImage.GetMetadata().format, 1, D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_COPY_DEST)) {
        return false;
    }

    // サブリソースデータの構築
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(m_pDevice->GetDevice(), convertedImage.GetImages(),
        convertedImage.GetImageCount(), convertedImage.GetMetadata(),
        subresources);

    // テクスチャのアップロード
    batch.Upload(m_equirectMap.GetResource(), 0, subresources.data(),
        static_cast<UINT>(subresources.size()));

    // PIXEL_SHADER_RESOURCEへ遷移
    batch.Transition(m_equirectMap.GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // SRVの作成
    DescriptorAllocation allocation = m_pPoolSRV->Allocate();

    D3D12_RESOURCE_DESC texDesc             = m_equirectMap.GetDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = texDesc.MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
    srvDesc.Texture2D.PlaneSlice            = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format                  = texDesc.Format;

    m_pDevice->GetDevice()->CreateShaderResourceView(
        m_equirectMap.GetResource(), &srvDesc, allocation.GetCPUHandle());

    m_equirectSrv = std::move(allocation);

    return true;
}

D3D12_GPU_DESCRIPTOR_HANDLE EnvironmentMap::GetEquirectSrvGpuHandle() const {
    if (m_equirectSrv.IsValid() && m_pPoolSRV) {
        return m_equirectSrv.GetGPUHandle();
    }

    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE EnvironmentMap::GetCubemapUavGpuHandle() const {
    if (m_cubemapUav.IsValid() && m_pPoolSRV) {
        return m_cubemapUav.GetGPUHandle();
    }

    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE EnvironmentMap::GetCubemapSrvGpuHandle() const {
    if (m_cubemapSrv.IsValid() && m_pPoolSRV) {
        return m_cubemapSrv.GetGPUHandle();
    }

    return {};
}
