#include "Engine/Resource/ShaderResourceTexture.h"

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/DxDebug.h"

ShaderResourceTexture::ShaderResourceTexture() : m_pPoolSRV(nullptr) {}

ShaderResourceTexture::~ShaderResourceTexture() { Term(); }

bool ShaderResourceTexture::InitFromImage(ID3D12Device* pDevice,
    DescriptorPool* pPoolSRV, const ImageAsset& image,
    DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (!pDevice || !pPoolSRV || !image.IsValid()) {
        return false;
    }

    // 既存リソースの破棄
    Term();

    m_pPoolSRV = pPoolSRV;

    // TextureResourceの作成
    {
        // 画像データの読み込み
        DirectX::ScratchImage srcImage;
        CHECK_HR(pDevice, DirectX::LoadFromWICMemory(image.imageData.data(),
                              image.imageData.size(), DirectX::WIC_FLAGS_NONE,
                              nullptr, srcImage));

        // ミップチェーン生成
        DirectX::ScratchImage mipChain;
        const DirectX::TexMetadata& srcMeta = srcImage.GetMetadata();

        auto hr = DirectX::GenerateMipMaps(srcImage.GetImages(),
            srcImage.GetImageCount(), srcMeta,
            DirectX::TEX_FILTER_DEFAULT | DirectX::TEX_FILTER_WRAP, 0,
            mipChain);
        if (FAILED(hr)) {
            mipChain = std::move(srcImage);
        }

        const DirectX::TexMetadata& mipMeta = mipChain.GetMetadata();

        // TextureResourceの初期化
        bool result = m_texture.InitAsTexture2D(pDevice, mipMeta.width,
            mipMeta.height, mipMeta.format, mipMeta.mipLevels,
            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
        if (!result) {
            return false;
        }

        // すべてのmip，配列スライスをアップロード
        std::vector<D3D12_SUBRESOURCE_DATA> subresources;
        DirectX::PrepareUpload(pDevice, mipChain.GetImages(),
            mipChain.GetImageCount(), mipMeta, subresources);

        // テクスチャのアップロード
        batch.Upload(m_texture.GetResource(), 0, subresources.data(),
            static_cast<UINT>(subresources.size()));

        // PIXEL_SHADER_RESOURCEへ遷移
        batch.Transition(m_texture.GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    // SRVの作成
    {
        DescriptorAllocation allocation = m_pPoolSRV->Allocate();

        D3D12_RESOURCE_DESC texDesc             = m_texture.GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip       = 0;
        srvDesc.Texture2D.MipLevels             = texDesc.MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
        srvDesc.Texture2D.PlaneSlice            = 0;
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        // フォーマットの決定
        if (image.isSRGB) {
            srvDesc.Format = DirectX::MakeSRGB(texDesc.Format);
        } else {
            srvDesc.Format = texDesc.Format;
        }

        pDevice->CreateShaderResourceView(
            m_texture.GetResource(), &srvDesc, allocation.GetCPUHandle());

        m_srvs.push_back(std::move(allocation));
    }

    return true;
}

bool ShaderResourceTexture::InitSolidColorRGBA8(ID3D12Device* pDevice,
    DescriptorPool* pPoolSRV, uint32_t color,
    DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (!pDevice || !pPoolSRV) {
        return false;
    }

    // 既存リソースの破棄
    Term();

    m_pPoolSRV = pPoolSRV;

    // リソースの作成
    bool result =
        m_texture.InitAsTexture2D(pDevice, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1,
            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
    if (!result) {
        return false;
    }

    // サブリソースデータの設定
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData                  = &color;
    subresourceData.RowPitch               = sizeof(uint32_t);
    subresourceData.SlicePitch             = sizeof(uint32_t);

    // アップロード
    batch.Upload(m_texture.GetResource(), 0, &subresourceData, 1);

    // リソースバリアの遷移
    batch.Transition(m_texture.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // SRVの作成
    DescriptorAllocation allocation = m_pPoolSRV->Allocate();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = 1;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
    srvDesc.Texture2D.PlaneSlice            = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    pDevice->CreateShaderResourceView(
        m_texture.GetResource(), &srvDesc, allocation.GetCPUHandle());

    m_srvs.push_back(std::move(allocation));

    return true;
}

void ShaderResourceTexture::Term() {
    m_srvs.clear();
    m_texture.Term();
    m_pPoolSRV = nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceTexture::GetDefaultSrvGpu() const {
    if (m_srvs.empty() || !m_srvs.front().IsValid() || !m_pPoolSRV) {
        return {};
    }
    return m_srvs.front().GetGPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceTexture::GetDefaultSrvCpu() const {
    if (m_srvs.empty() || !m_srvs.front().IsValid() || !m_pPoolSRV) {
        return {};
    }
    return m_srvs.front().GetCPUHandle();
}