#include "Engine/MaterialGPU.h"

MaterialGPU::MaterialGPU()
    : m_constantBuffer(),
      m_pTextureManager(nullptr),
      m_constants(),
      m_baseColorIndex(std::nullopt),
      m_metallicRoughnessIndex(std::nullopt),
      m_normalIndex(std::nullopt),
      m_emissiveIndex(std::nullopt),
      m_occlusionIndex(std::nullopt) {
    m_constants.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_constants.metallic  = 0.0f;
    m_constants.roughness = 0.5f;
    m_constants.emissive  = { 0.0f, 0.0f, 0.0f };
    m_constants.occlusion = 1.0f;
}

MaterialGPU::~MaterialGPU() { Term(); }

// 初期化処理，MaterialAssetからGPUリソースを作成
bool MaterialGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    TextureManager* pTextureManager, const MaterialAsset& materialAsset) {
    // 引数チェック
    if (pDevice == nullptr || pPoolCBV == nullptr ||
        pTextureManager == nullptr) {
        return false;
    }

    m_pTextureManager = pTextureManager;

    // 定数バッファの作成
    if (!m_constantBuffer.Init(
            pDevice, pPoolCBV, sizeof(shader::MaterialConstants))) {
        return false;
    }

    // マテリアル定数の設定
    m_constants.baseColor = materialAsset.baseColor;
    m_constants.metallic  = materialAsset.metallicFactor;
    m_constants.roughness = materialAsset.roughnessFactor;
    m_constants.emissive  = materialAsset.emissiveFactor;
    m_constants.occlusion = materialAsset.occlusionFactor;

    // 定数バッファの更新
    m_constantBuffer.Update(&m_constants, sizeof(shader::MaterialConstants));

    // テクスチャインデックスを設定
    if (materialAsset.baseColorTexture.IsValid()) {
        m_baseColorIndex = materialAsset.baseColorTexture.index;
    }
    if (materialAsset.metallicRoughnessTexture.IsValid()) {
        m_metallicRoughnessIndex = materialAsset.metallicRoughnessTexture.index;
    }
    if (materialAsset.occlusionTexture.IsValid()) {
        m_occlusionIndex = materialAsset.occlusionTexture.index;
    }
    if (materialAsset.normalTexture.IsValid()) {
        m_normalIndex = materialAsset.normalTexture.index;
    }
    if (materialAsset.emissiveTexture.IsValid()) {
        m_emissiveIndex = materialAsset.emissiveTexture.index;
    }

    return true;
}

// 終了処理
void MaterialGPU::Term() {
    m_constantBuffer.Term();
    m_pTextureManager        = nullptr;
    m_baseColorIndex         = std::nullopt;
    m_metallicRoughnessIndex = std::nullopt;
    m_normalIndex            = std::nullopt;
}

// 描画で使うためのテクスチャを取得する
// テクスチャがない場合はデフォルトテクスチャを返す
ShaderResourceTexture* MaterialGPU::GetTexture(TextureUsage usage) const {
    if (m_pTextureManager == nullptr) {
        return nullptr;
    }

    uint32_t index;
    switch (usage) {
        case TextureUsage::BaseColor:
            if (!m_baseColorIndex.has_value()) {
                return m_pTextureManager->GetWhiteDefault();
            }
            index = m_baseColorIndex.value();
            break;
        case TextureUsage::MetallicRoughness:
            if (!m_metallicRoughnessIndex.has_value()) {
                // Factor値を使うためRMA Defaultではなく白色を返す
                return m_pTextureManager->GetWhiteDefault();
            }
            index = m_metallicRoughnessIndex.value();
            break;
        case TextureUsage::Emissive:
            if (!m_emissiveIndex.has_value()) {
                return m_pTextureManager->GetWhiteDefault();
            }
            index = m_emissiveIndex.value();
            break;
        case TextureUsage::Normal:
            if (!m_normalIndex.has_value()) {
                return m_pTextureManager->GetNormalFlat();
            }
            index = m_normalIndex.value();
            break;
        case TextureUsage::Occlusion:
            if (!m_occlusionIndex.has_value()) {
                return m_pTextureManager->GetWhiteDefault();
            }
            index = m_occlusionIndex.value();
            break;
    }

    return m_pTextureManager->GetTexture(index);
}