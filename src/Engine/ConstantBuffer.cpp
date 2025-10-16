#include "Engine/ConstantBuffer.h"

#include "Engine/DescriptorPool.h"

// コンストラクタ
ConstantBuffer::ConstantBuffer()
    : m_pCB(nullptr), m_pPool(nullptr), m_pMappedData(nullptr) {}

// デストラクタ
ConstantBuffer::~ConstantBuffer() { Term(); }

// 初期化処理
bool ConstantBuffer::Init(
    ID3D12Device* pDevice, DescriptorPool* pPool, size_t size) {
    // 引数チェック
    if (pDevice == nullptr || pPool == nullptr || size == 0) {
        return false;
    }

    m_pPool = pPool;
    m_index = m_pPool->Allocate();

    // 定数バッファのサイズを256の倍数に切り上げる
    // ~(align - 1) は，2^8-1 のビット反転なので，下位8ビットが0，
    // 元の数字に256を足してマスクすることで256の倍数に切り上げられる
    const size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    UINT64 sizeAligned = (size + align - 1) & ~(align - 1);

    // 定数バッファの生成
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                  = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    prop.VisibleNodeMask       = 1;
    prop.CreationNodeMask      = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment           = 0;
    desc.Width               = sizeAligned;
    desc.Height              = 1;
    desc.DepthOrArraySize    = 1;
    desc.MipLevels           = 1;
    desc.Format              = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count    = 1;
    desc.SampleDesc.Quality  = 0;
    desc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    // リソースの生成
    auto hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(m_pCB.GetAddressOf()));
    if (FAILED(hr)) {
        m_pPool->Free(m_index);
        return false;
    }

    // メモリマッピング
    hr = m_pCB->Map(0, nullptr, &m_pMappedData);
    if (FAILED(hr)) {
        m_pPool->Free(m_index);
        m_pCB.Reset();
        return false;
    }

    m_GPUAddress = m_pCB->GetGPUVirtualAddress();

    D3D12_CONSTANT_BUFFER_VIEW_DESC CBVdesc = {};
    CBVdesc.BufferLocation                  = m_GPUAddress;
    CBVdesc.SizeInBytes = static_cast<UINT>(m_pCB->GetDesc().Width);

    pDevice->CreateConstantBufferView(&CBVdesc, pPool->GetCPUHandle(m_index));

    return true;
}

// 終了処理
void ConstantBuffer::Term() {
    // メモリマッピングの解除
    if (m_pCB != nullptr) {
        m_pCB->Unmap(0, nullptr);
        m_pCB.Reset();
    }

    // ビューの破棄
    if (m_pPool != nullptr) {
        m_pPool->Free(m_index);
        m_pPool = nullptr;
        m_index = 0;
    }

    m_pMappedData = nullptr;
}

// 定数バッファの更新
void ConstantBuffer::Update(const void* pData, size_t size) {
    if (m_pMappedData != nullptr && pData != nullptr && size > 0) {
        memcpy(m_pMappedData, pData, size);
    }
}