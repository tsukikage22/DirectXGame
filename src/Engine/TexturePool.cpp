
#include "Engine/TexturePool.h"

TexturePool::TexturePool() : m_pDevice(nullptr), m_pPoolSRV(nullptr) {}

// 初期化
bool TexturePool::Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV) {
    // 引数チェック
    if (!pDevice || !pPoolSRV) {
        return false;
    }

    m_pDevice  = pDevice;
    m_pPoolSRV = pPoolSRV;

    return true;
}

// 終了処理
void TexturePool::Term() {
    m_textures.clear();
    if (m_pDefaultTexture) {
        m_pDefaultTexture->Term();
        m_pDefaultTexture.reset();
    }
    m_pPoolSRV = nullptr;
    m_pDevice  = nullptr;
}

// ImageAsset配列からテクスチャを生成
bool TexturePool::CreateFromImages(const std::vector<ImageAsset>& images,
    DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (images.empty()) {
        return false;
    }

    // テクスチャ生成
    m_textures.reserve(images.size());
    for (int i = 0; i < images.size(); i++) {
        if (!images[i].IsValid()) {
            return false;
        }

        // ImageAssetからテクスチャを生成
        TextureGPU texture;
        texture.Init(m_pDevice, m_pPoolSRV, batch, images[i]);
        m_textures.push_back(std::make_unique<TextureGPU>(std::move(texture)));
    }

    return true;
}

// デフォルトテクスチャの作成
bool TexturePool::CreateDefaultTexture(DirectX::ResourceUploadBatch& batch) {
    if (!m_pDevice || !m_pPoolSRV) {
        return false;
    }

    // すでに作成済みか確認
    if (m_pDefaultTexture) {
        return true;
    }

    // 1x1ピクセルの白色テクスチャ（RGBA8）
    const uint8_t white = 0xFFFFFFFF;

    // テクスチャリソースの設定
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width               = 1;
    texDesc.Height              = 1;
    texDesc.DepthOrArraySize    = 1;
    texDesc.MipLevels           = 1;
    texDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count    = 1;
    texDesc.SampleDesc.Quality  = 0;
    texDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    // ヒーププロパティの設定
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type                  = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask      = 1;
    heapProps.VisibleNodeMask       = 1;

    // この先の処理の意味について
    // 1. アップロードするテクスチャのコピー先としてdefaultヒープを作成する
    // 2. ResourceUploadBatchを使ってテクスチャをアップロードする
    // 3. リソースバリアの状態遷移を行いピクセルシェーダーが参照できるように

    // テクスチャリソースの作成
    engine::ComPtr<ID3D12Resource> pTexture;
    auto hr = m_pDevice->CreateCommittedResource(&heapProps,
        D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(pTexture.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    // サブリソースデータの設定
    // サブリソースとは：テクスチャをGPUに渡すための指定
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData                  = &white;
    subresourceData.RowPitch               = sizeof(uint32_t);
    subresourceData.SlicePitch             = sizeof(uint32_t);

    // テクスチャデータのアップロード
    batch.Upload(pTexture.Get(), 0, &subresourceData, 1);

    // PIXEL_SHADER_RESOURCE状態に遷移
    batch.Transition(pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // TextureGPUの作成
    m_pDefaultTexture = std::make_unique<TextureGPU>();
    if (!m_pDefaultTexture->InitFromResource(
            m_pDevice, m_pPoolSRV, pTexture.Get())) {
        // 失敗時のリソース解放
        m_pDefaultTexture.reset();
        return false;
    }

    return true;
}

// 指定したインデックスのテクスチャを取得，-1の場合はデフォルトテクスチャを返す
TextureGPU* TexturePool::GetTexture(int index) {
    if (index == -1) {
        return m_pDefaultTexture.get();
    } else if (index < 0 || index >= m_textures.size()) {
        return nullptr;
    } else {
        return m_textures[index].get();
    }
}