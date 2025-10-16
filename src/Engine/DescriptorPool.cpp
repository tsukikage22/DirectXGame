#include "Engine/DescriptorPool.h"

#include <stdexcept>

bool DescriptorPool::Create(ID3D12Device* pDevice,
    D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags,
    uint32_t capacity, DescriptorPool** outPool) {
    // 引数チェック
    if (pDevice == nullptr || outPool == nullptr) {
        return false;
    }

    // プールのインスタンス生成
    auto pool = new (std::nothrow) DescriptorPool();
    if (pool == nullptr) {
        return false;
    }

    // ディスクリプタヒープ生成
    try {
        pool->m_pHeap = std::make_unique<DirectX::DescriptorHeap>(
            pDevice, type, flags, capacity);
    } catch (const std::exception&) {
        pool->m_pHeap.reset();
        return false;
    }

    pool->m_capacity = capacity;
    pool->m_shaderVisible =
        (flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0;

    // フリーリスト作成
    pool->m_free.reserve(capacity);
    for (uint32_t i = 0; i < capacity; i++) {
        pool->m_free.push_back(i);
    }

    // 正常終了
    *outPool = pool;
    return true;
}

// プールへの割り当て
uint32_t DescriptorPool::Allocate() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_free.empty()) throw std::runtime_error("Descriptor fully allocated");
    uint32_t index = m_free.back();
    m_free.pop_back();
    return index;
}

// 割り当ての解放
void DescriptorPool::Free(uint32_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (index >= m_capacity) return;
    m_free.push_back(index);
}

// CPUハンドル取得
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorPool::GetCPUHandle(uint32_t index) const {
    return m_pHeap->GetCpuHandle(index);
}

// GPUハンドル取得
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorPool::GetGPUHandle(uint32_t index) const {
    return m_pHeap->GetGpuHandle(index);
}

uint32_t DescriptorPool::Capacity() const { return m_capacity; }

uint32_t DescriptorPool::FreeCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<uint32_t>(m_free.size());
}

DescriptorPool::DescriptorPool() : m_capacity(0), m_shaderVisible(false) {}

DescriptorPool::~DescriptorPool() {
    m_pHeap.reset();
    m_free.clear();
    m_capacity = 0;
}
