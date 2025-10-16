#include "Engine/MaterialGPU.h"

MaterialGPU::MaterialGPU()
    : m_constantBuffer(), m_pTexturePool(nullptr), m_constants() {
    m_constants.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_constants.metallic  = 0.0f;
    m_constants.roughness = 0.5f;
}

MaterialGPU::~MaterialGPU() { Term(); }

// 初期化処理，MaterialAssetからGPUリソースを作成
bool MaterialGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    TexturePool* pTexturePool,
    DirectX::ResourceUploadBatch& resourceUploadBatch,
    const MaterialAsset& materialAsset) {
    // 引数チェック
    if (pDevice == nullptr || pPoolCBV == nullptr || pTexturePool == nullptr) {
        return false;
    }

    // 定数バッファの作成
    if (!m_constantBuffer.Init(
            pDevice, pPoolCBV, sizeof(shader::MaterialConstants))) {
        return false;
    }

    // マテリアル定数の設定
    m_constants.baseColor = materialAsset.baseColor;
    m_constants.metallic  = materialAsset.metallic;
    m_constants.roughness = materialAsset.roughness;

    // 定数バッファの更新
    m_constantBuffer.Update(&m_constants, sizeof(shader::MaterialConstants));

    // テクスチャインデックスを設定
    if (materialAsset.baseColorTexture.IsValid()) {
        m_baseColorIndex = materialAsset.baseColorTexture.index;
    }
    if (materialAsset.metallicTexture.IsValid()) {
        m_metallicIndex = materialAsset.metallicTexture.index;
    }
    if (materialAsset.roughnessTexture.IsValid()) {
        m_roughnessIndex = materialAsset.roughnessTexture.index;
    }
    if (materialAsset.normalTexture.IsValid()) {
        m_normalIndex = materialAsset.normalTexture.index;
    }

    return true;
}

// 終了処理
void MaterialGPU::Term() {
    m_constantBuffer.Term();
    m_pTexturePool   = nullptr;
    m_baseColorIndex = -1;
    m_metallicIndex  = -1;
    m_roughnessIndex = -1;
    m_normalIndex    = -1;
}

// 描画で使うためのテクスチャを取得する
TextureGPU* MaterialGPU::GetTexture(TextureUsage usage) const {
    if (m_pTexturePool == nullptr) {
        return nullptr;
    }

    int index = -1;
    switch (usage) {
        case TextureUsage::BaseColor:
            index = m_baseColorIndex;
            break;
        case TextureUsage::Metallic:
            index = m_metallicIndex;
            break;
        case TextureUsage::Roughness:
            index = m_roughnessIndex;
            break;
        case TextureUsage::Normal:
            index = m_normalIndex;
            break;
    }

    return m_pTexturePool->GetTexture(index);
}