#include "Engine/TextureManager.h"

TextureManager::TextureManager() : m_pDevice(nullptr), m_pPoolSRV(nullptr) {}

// 初期化
bool TextureManager::Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV) {
    // 引数チェック
    if (!pDevice || !pPoolSRV) {
        return false;
    }

    m_pDevice  = pDevice;
    m_pPoolSRV = pPoolSRV;

    return true;
}

// 終了処理
void TextureManager::Term() {
    // デフォルトテクスチャの解放
    m_pDefaultWhiteTexture.reset();
    m_pDefaultNormalFlatTexture.reset();
    m_pDefaultRmaTexture.reset();

    // テクスチャリソースの解放
    for (auto& t : m_textures) {
        t.Term();
    }
    m_textures.clear();

    m_pPoolSRV = nullptr;
    m_pDevice  = nullptr;
}

void TextureManager::BuildTexturesFromModelAsset(
    ModelAsset& modelAsset, DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (!modelAsset.IsValid()) {
        return;
    }

    // ImageAsset配列からテクスチャを生成
    std::vector<TextureHandle> textureHandles;
    for (const auto& image : modelAsset.images) {
        uint32_t index = CreateFromImageAsset(image, batch);
        TextureHandle textureHandle{ index };
        textureHandles.push_back(textureHandle);
    }

    // マテリアルのテクスチャハンドルを解決
    for (auto& material : modelAsset.materials) {
        auto Resolve = [&](int localIndex, TextureHandle& outHandle) {
            if (localIndex >= 0 &&
                static_cast<size_t>(localIndex) < textureHandles.size()) {
                outHandle = textureHandles[localIndex];
            } else {
                // 無効の場合は初期値（無効ハンドル）のまま
            }
        };
        Resolve(material.baseColorLocalTextureIndex, material.baseColorTexture);
        Resolve(material.normalLocalTextureIndex, material.normalTexture);
        Resolve(material.occlusionLocalTextureIndex, material.occlusionTexture);
        Resolve(material.emissiveLocalTextureIndex, material.emissiveTexture);
    }
}

// ImageAsset配列からテクスチャを生成
uint32_t TextureManager::CreateFromImageAsset(
    const ImageAsset& image, DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (!image.IsValid()) {
        return UINT32_MAX;
    }

    // TextureResourceを生成
    // フォーマットチェック
    // GLBの埋め込み画像想定で，png, jpgを対象にする
    if (image.format != "png" && image.format != "jpg" &&
        image.format != "jpeg") {
        return UINT32_MAX;
    }

    ShaderResourceTexture shaderResourceTexture;
    if (!shaderResourceTexture.InitFromImage(
            m_pDevice, m_pPoolSRV, image, batch)) {
        return UINT32_MAX;
    }

    // 配列に追加
    uint32_t handle = static_cast<uint32_t>(m_textures.size());
    m_textures.push_back(std::move(shaderResourceTexture));

    return handle;
}

// 単色テクスチャの生成
bool TextureManager::CreateSolidColorTexture(
    DirectX::ResourceUploadBatch& batch, uint32_t color,
    ShaderResourceTexture& outTexture) {
    return outTexture.InitSolidColorRGBA8(m_pDevice, m_pPoolSRV, color, batch);
}

// 指定したインデックスのテクスチャを取得
ShaderResourceTexture* TextureManager::GetTexture(uint32_t index) {
    TextureHandle handle{ index };
    if (!handle.IsValid() ||
        handle.index >= static_cast<uint32_t>(m_textures.size())) {
        return nullptr;
    } else {
        return &m_textures[handle.index];
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvGPUHandle(
    TextureHandle handle) const {
    if (!handle.IsValid() ||
        handle.index >= static_cast<uint32_t>(m_textures.size())) {
        return {};
    } else {
        return m_textures[handle.index].GetDefaultSrvGpu();
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetSrvCpuHandle(
    TextureHandle handle) const {
    if (!handle.IsValid() ||
        handle.index >= static_cast<uint32_t>(m_textures.size())) {
        return {};
    } else {
        return m_textures[handle.index].GetDefaultSrvCpu();
    }
}

bool TextureManager::CreateDefaultTextures(
    DirectX::ResourceUploadBatch& batch) {
    // デフォルトテクスチャの生成
    m_pDefaultWhiteTexture      = std::make_unique<ShaderResourceTexture>();
    m_pDefaultNormalFlatTexture = std::make_unique<ShaderResourceTexture>();
    m_pDefaultRmaTexture        = std::make_unique<ShaderResourceTexture>();

    // 白色ベースカラー（1.0, 1.0, 1.0），A=0xFF, R=0xFF, G=0xFF, B=0xFF
    bool result =
        CreateSolidColorTexture(batch, 0xFFFFFFFF, *m_pDefaultWhiteTexture);
    if (!result) {
        return false;
    }

    // 法線マップ（0.5, 0.5, 1.0），A=0xFF, R=0x80, G=0x80, B=0xFF
    result = CreateSolidColorTexture(
        batch, 0xFF8080FF, *m_pDefaultNormalFlatTexture);
    if (!result) {
        return false;
    }

    // RMAテクスチャ（1.0, 0.0, 1.0），A=0xFF, R=0xFF, G=0x00, B=0xFF
    result = CreateSolidColorTexture(batch, 0xFFFF00FF, *m_pDefaultRmaTexture);
    if (!result) {
        return false;
    }

    return true;
}

ShaderResourceTexture* TextureManager::GetWhiteDefault() const {
    return m_pDefaultWhiteTexture.get();
}

ShaderResourceTexture* TextureManager::GetNormalFlat() const {
    return m_pDefaultNormalFlatTexture.get();
}

ShaderResourceTexture* TextureManager::GetRmaDefault() const {
    return m_pDefaultRmaTexture.get();
}