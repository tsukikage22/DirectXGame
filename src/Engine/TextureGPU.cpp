#include "Engine/TextureGPU.h"

// コンストラクタ
TextureGPU::TextureGPU() : m_pTexture(nullptr), m_pPool(nullptr), m_index(0) {}

// デストラクタ
TextureGPU::~TextureGPU() { Term(); }

// ImageAssetからテクスチャを生成する
bool TextureGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPool,
    DirectX::ResourceUploadBatch& resourceUploadBatch,
    const ImageAsset& imageAsset) {
    // 引数チェック
    if (!pDevice || !pPool) {
        return false;
    }

    m_pPool = pPool;
    m_index = m_pPool->Allocate();

    // png/jpegの場合（gltf標準，それ以外の場合はまた考える）
    if (imageAsset.format != "png" && imageAsset.format != "jpg" &&
        imageAsset.format != "jpeg") {
        return false;
    }

    // テクスチャリソースの生成
    auto hr = DirectX::CreateWICTextureFromMemory(pDevice, resourceUploadBatch,
        imageAsset.imageData.data(), imageAsset.imageData.size(),
        m_pTexture.GetAddressOf(), true);
    if (FAILED(hr)) {
        return false;
    }

    // SRVの生成
    // glbを読み込むことを想定しているため，TEXTURE2Dのみ対応，DDSに対応する場合は分岐が必要
    D3D12_RESOURCE_DESC texDesc             = m_pTexture->GetDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = texDesc.Format;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = texDesc.MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
    srvDesc.Texture2D.PlaneSlice            = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    pDevice->CreateShaderResourceView(
        m_pTexture.Get(), &srvDesc, m_pPool->GetCPUHandle(m_index));

    return true;
}

// 終了処理，リソースの開放
void TextureGPU::Term() {
    if (m_pPool && m_index != 0) {
        m_pPool->Free(m_index);
        m_index = 0;
        m_pPool = nullptr;
    }
    m_pTexture.Reset();
    m_pPool = nullptr;
}

// 作成済みリソースから初期化（デフォルトテクスチャを想定）
bool TextureGPU::InitFromResource(ID3D12Device* pDevice,
    DescriptorPool* pPoolSRV, ID3D12Resource* pResource) {
    // 引数チェック
    if (!pDevice || !pPoolSRV || !pResource) {
        return false;
    }

    m_pPool = pPoolSRV;
    m_index = m_pPool->Allocate();

    m_pTexture = pResource;

    // SRVの生成
    D3D12_RESOURCE_DESC texDesc             = m_pTexture->GetDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = texDesc.Format;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = texDesc.MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
    srvDesc.Texture2D.PlaneSlice            = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    pDevice->CreateShaderResourceView(
        m_pTexture.Get(), &srvDesc, m_pPool->GetCPUHandle(m_index));

    return true;
}

// SRVのGPUハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE TextureGPU::GetGPUHandle() const {
    if (!m_pPool || m_index == 0) {
        return {};
    }
    return m_pPool->GetGPUHandle(m_index);
}