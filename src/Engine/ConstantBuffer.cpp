#include "Engine/ConstantBuffer.h"

#include "Engine/DescriptorPool.h"

// コンストラクタ
ConstantBuffer::ConstantBuffer()
    : m_pPool(nullptr), m_pMappedData(nullptr), m_index(0) {}

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
    m_size  = size;

    // 定数バッファのサイズを256の倍数に切り上げる
    // ~(align - 1) は，2^8-1 のビット反転なので，下位8ビットが0，
    // 元の数字に256を足してマスクすることで256の倍数に切り上げられる
    const size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    UINT64 sizeAligned = (size + align - 1) & ~(align - 1);

    // バッファの作成とメモリマッピング
    if (!m_buffer.CreateDynamic(pDevice, sizeAligned)) {
        m_pPool->Free(m_index);
        return false;
    }

    m_pMappedData = m_buffer.GetMappedPtr();
    if (m_pMappedData == nullptr) {
        m_buffer.Term();
        m_pPool->Free(m_index);
        return false;
    }

    m_GPUAddress = m_buffer.GetGPUVirtualAddress();

    // CBVの作成
    D3D12_CONSTANT_BUFFER_VIEW_DESC CBVdesc = {};
    CBVdesc.BufferLocation                  = m_GPUAddress;
    CBVdesc.SizeInBytes                     = static_cast<UINT>(sizeAligned);

    pDevice->CreateConstantBufferView(&CBVdesc, pPool->GetCPUHandle(m_index));

    return true;
}

// 終了処理
void ConstantBuffer::Term() {
    // メモリマッピングの解除
    m_buffer.Term();

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
    if (m_pMappedData == nullptr || pData == nullptr || size == 0) {
        return;
    }
    if (size > m_size) {
        assert(false && "buffer overflow");
        return;
    }
    memcpy(m_pMappedData, pData, size);
}