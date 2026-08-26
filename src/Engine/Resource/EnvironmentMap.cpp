#include "Engine/Resource/EnvironmentMap.h"

#include <DirectXPackedVector.h>
#include <DirectXTex.h>
#include <Windows.h>

#include <cmath>
#include <sstream>
#include <vector>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"

namespace /* anonymous */ {
constexpr static float kDefaultLuminance =
    30.0f;  // デフォルトキューブマップの輝度

/// @brief
/// equirect画像が水平面に作る照度を求める（HDRIの値をcd/m²とみなした相対値）
float ComputeUpperHemisphereIlluminance(const DirectX::ScratchImage& image) {
    const auto& meta = image.GetMetadata();
    double sum       = 0.0;

    DirectX::EvaluateImage(*image.GetImage(0, 0, 0),
        [&](const DirectX::XMVECTOR* pixels, size_t width, size_t y) {
            // 行yが天頂角θに対応する（y=0が天頂）
            const double theta =
                (static_cast<double>(y) + 0.5) / meta.height * DirectX::XM_PI;

            // 下半球は水平面を照らさない
            if (theta >= DirectX::XM_PIDIV2) {
                return;
            }

            double rowSum = 0.0;
            for (size_t x = 0; x < width; ++x) {
                DirectX::XMFLOAT3 c;
                DirectX::XMStoreFloat3(&c, pixels[x]);
                // Rec.709の輝度
                rowSum += 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z;
            }

            // cosθ（ランバート則）× sinθ（立体角のヤコビアン）
            sum += rowSum * std::cos(theta) * std::sin(theta);
        });

    // dω = sinθ dθ dφ，dθ = π/height，dφ = 2π/width
    return static_cast<float>(sum * 2.0 * DirectX::XM_PI * DirectX::XM_PI /
                              (meta.width * meta.height));
}

/// @brief デフォルトキューブマップの作成
bool BuildDefaultCubemap(GraphicsDevice* pDevice, TextureResource& cubeMap,
    DescriptorAllocation& srv, DirectX::ResourceUploadBatch& batch) {
    if (!pDevice) {
        return false;
    }
    // デフォルトキューブマップは，1x1の6面のテクスチャで，輝度30cd/m²のマゼンタ色を持つ
    // リソースの作成
    if (!cubeMap.InitAsTexture2DArray(pDevice->GetDevice(), 1, 1,
            DXGI_FORMAT_R16G16B16A16_FLOAT, 6, 1, D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_COPY_DEST)) {
        OutputDebugStringW(L"Failed to create default cubemap resource.\n");
        return false;
    }

    const DirectX::PackedVector::XMHALF4 kMagenta = { kDefaultLuminance, 0.0f,
        kDefaultLuminance, 1.0f };
    D3D12_SUBRESOURCE_DATA faces[6]               = {};
    for (auto& f : faces) {
        f.pData      = &kMagenta;
        f.RowPitch   = sizeof(kMagenta);
        f.SlicePitch = sizeof(kMagenta);
    }
    batch.Upload(cubeMap.GetResource(), 0, faces, 6);
    batch.Transition(cubeMap.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // SRVの作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip     = 0;
    srvDesc.TextureCube.MipLevels           = 1;
    srvDesc.Format                          = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv                             = pDevice->CbvSrvUavPool()->Allocate();
    pDevice->GetDevice()->CreateShaderResourceView(
        cubeMap.GetResource(), &srvDesc, srv.GetCPUHandle());

    return true;
}

/// @brief irradiance mapをゼロクリアする
/// @param resource irradiance mapのリソース
/// @param batch リソースアップロード用バッチ
void ClearIrradianceMap(TextureResource& resource, uint32_t size,
    DXGI_FORMAT format, DirectX::ResourceUploadBatch& batch) {
    const UINT kPixelSize =
        static_cast<UINT>(DirectX::BitsPerPixel(format) / 8);
    const UINT kRowPitch   = size * kPixelSize;
    const UINT kSlicePitch = kRowPitch * size;
    const std::vector<uint8_t> zeroData(kSlicePitch, 0);

    D3D12_SUBRESOURCE_DATA faces[6] = {};
    for (auto& f : faces) {
        f.pData      = zeroData.data();
        f.RowPitch   = kRowPitch;
        f.SlicePitch = kSlicePitch;
    }
    batch.Transition(resource.GetResource(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COPY_DEST);
    batch.Upload(resource.GetResource(), 0, faces, 6);
    batch.Transition(resource.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

/// @brief キューブマップ用リソースとUAV/SRVの作成
bool CreateCubemapResourceAndViews(GraphicsDevice* pDevice,
    DescriptorPool* pPool, uint32_t size, DXGI_FORMAT format,
    TextureResource& resource, DescriptorAllocation& uav,
    DescriptorAllocation& srv) {
    // キューブマップ用リソースの作成
    if (!resource.InitAsTexture2DArray(pDevice->GetDevice(), size, size, format,
            6, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
        OutputDebugStringW(L"Failed to create cubemap resource.\n");
        return false;
    }

    // UAVの作成
    uav                                      = pPool->Allocate();
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.MipSlice        = 0;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize       = 6;
    uavDesc.Format                         = format;
    pDevice->GetDevice()->CreateUnorderedAccessView(
        resource.GetResource(), nullptr, &uavDesc, uav.GetCPUHandle());

    // SRVの作成
    srv                                     = pPool->Allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip     = 0;
    srvDesc.TextureCube.MipLevels           = 1;
    srvDesc.Format                          = format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    pDevice->GetDevice()->CreateShaderResourceView(
        resource.GetResource(), &srvDesc, srv.GetCPUHandle());
    return true;
}

}  // namespace

bool EnvironmentMap::Init(
    GraphicsDevice* pDevice, DirectX::ResourceUploadBatch& batch) {
    Term();
    if (!pDevice) {
        return false;
    }

    m_pDevice  = pDevice;
    m_pPoolSRV = pDevice->CbvSrvUavPool();

    // キューブマップ用リソース，UAV，SRVの作成
    if (!CreateCubemapResourceAndViews(m_pDevice, m_pPoolSRV, kCubeMapSize,
            kCubemapFormat, m_cubeMap, m_cubemapUav, m_cubemapSrv)) {
        return false;
    }

    // irradiance map用リソース，UAV，SRVの作成
    if (!CreateCubemapResourceAndViews(m_pDevice, m_pPoolSRV,
            kIrradianceMapSize, kIrradianceMapFormat, m_irradianceMap,
            m_irradianceUav, m_irradianceSrv)) {
        return false;
    }

    // デフォルトキューブマップの作成
    if (!BuildDefaultCubemap(
            m_pDevice, m_defaultCubeMap, m_defaultSrv, batch)) {
        OutputDebugStringW(L"Failed to build default cubemap.\n");
        return false;
    }

    // irradiance mapのゼロクリア
    ClearIrradianceMap(
        m_irradianceMap, kIrradianceMapSize, kIrradianceMapFormat, batch);

    return true;
}

void EnvironmentMap::Term() {
    m_canUseCubemap = false;
    m_equirectMap.Term();
    m_cubeMap.Term();
    m_defaultCubeMap.Term();
    m_irradianceMap.Term();
    m_equirectSrv   = {};
    m_cubemapUav    = {};
    m_cubemapSrv    = {};
    m_irradianceUav = {};
    m_irradianceSrv = {};
    m_defaultSrv    = {};
    m_pPoolSRV      = nullptr;
    m_pDevice       = nullptr;
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

    // 水平面の照度を計算
    float illuminance = ComputeUpperHemisphereIlluminance(clampedImage);
    std::wstringstream ss;
    ss << L"Upper hemisphere illuminance: " << illuminance << L" lx\n";
    OutputDebugStringW(ss.str().c_str());

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
    m_equirectSrv = m_pPoolSRV->Allocate();

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
        m_equirectMap.GetResource(), &srvDesc, m_equirectSrv.GetCPUHandle());

    m_canUseCubemap =
        true;  // HDRIの読み込みに成功した場合はデフォルトでないキューブマップを使う

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
    if (!m_pPoolSRV) {
        return {};
    }

    if (m_defaultSrv.IsValid() && !m_canUseCubemap) {
        return m_defaultSrv.GetGPUHandle();
    } else if (m_cubemapSrv.IsValid()) {
        return m_cubemapSrv.GetGPUHandle();
    }

    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE EnvironmentMap::GetIrradianceUavGpuHandle() const {
    if (m_irradianceUav.IsValid() && m_pPoolSRV) {
        return m_irradianceUav.GetGPUHandle();
    }

    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE EnvironmentMap::GetIrradianceSrvGpuHandle() const {
    if (m_irradianceSrv.IsValid() && m_pPoolSRV) {
        return m_irradianceSrv.GetGPUHandle();
    }

    return {};
}
