#include "Engine/GPUBuffer.h"

bool GPUBuffer::CreateStatic(ID3D12Device* pDevice,
    ID3D12GraphicsCommandList* pCmdList, size_t size, const void* pInitData,
    D3D12_RESOURCE_STATES finalState) {
    // 引数チェック
    if (pDevice == nullptr || pCmdList == nullptr || size == 0 ||
        pInitData == nullptr) {
        return false;
    }

    m_Size = size;

    // バッファリソースの設定
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.VisibleNodeMask = 1;
    prop.CreationNodeMask = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = (UINT64)size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // DEFAULTヒープの作成
    auto hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(m_pRes.GetAddressOf()));
    if (FAILED(hr)) {
        Term();
        return false;
    }
    m_State = D3D12_RESOURCE_STATE_COPY_DEST;

    // UPLOADヒープの作成
    prop.Type = D3D12_HEAP_TYPE_UPLOAD;
    hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(m_pUpload.GetAddressOf()));
    if (FAILED(hr)) {
        Term();
        return false;
    }

    // 初期データの書き込み
    void* ptr = nullptr;
    hr = m_pUpload->Map(0, nullptr, &ptr);
    if (FAILED(hr) || ptr == nullptr) {
        Term();
        return false;
    }
    memcpy(ptr, pInitData, size);
    m_pUpload->Unmap(0, nullptr);

    // コピー記録
    pCmdList->CopyBufferRegion(m_pRes.Get(), 0, m_pUpload.Get(), 0, size);

    // ステートの遷移
    if (m_State != finalState) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = m_pRes.Get();
        barrier.Transition.StateBefore = m_State;
        barrier.Transition.StateAfter = finalState;
        barrier.Transition.Subresource =
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        pCmdList->ResourceBarrier(1, &barrier);
        m_State = finalState;
    }

    return true;
}

// アップロードバッファの破棄,フェンス完了後に呼び出す
void GPUBuffer::DiscardUpload() { m_pUpload.Reset(); }

// 終了処理
void GPUBuffer::Term() {
    m_pRes.Reset();
    m_pUpload.Reset();
    m_State = D3D12_RESOURCE_STATE_COMMON;
}

D3D12_GPU_VIRTUAL_ADDRESS GPUBuffer::GetGPUVirtualAddress() const {
    if (m_pRes == nullptr) {
        return 0;
    }
    return m_pRes->GetGPUVirtualAddress();
}
