#include "Engine/Core/DescriptorAllocation.h"

#include "Engine/Core/DescriptorPool.h"

// コンストラクタ
DescriptorAllocation::DescriptorAllocation(
    DescriptorPool* pPool, uint32_t index)
    : m_pPool(pPool), m_index(index) {}

// コンストラクタ（連続領域確保用）
DescriptorAllocation::DescriptorAllocation(
    DescriptorPool* pPool, uint32_t index, uint32_t count)
    : m_pPool(pPool), m_index(index), m_count(count) {}

// デストラクタ
DescriptorAllocation::~DescriptorAllocation() { Release(); }

// ムーブコンストラクタ
DescriptorAllocation::DescriptorAllocation(
    DescriptorAllocation&& other) noexcept
    : m_pPool(other.m_pPool), m_index(other.m_index), m_count(other.m_count) {
    other.m_pPool = nullptr;
    other.m_index = 0;
    other.m_count = 1;
}

// ムーブ代入演算子
DescriptorAllocation& DescriptorAllocation::operator=(
    DescriptorAllocation&& other) noexcept {
    if (this != &other) {
        Release();
        m_pPool       = other.m_pPool;
        m_index       = other.m_index;
        m_count       = other.m_count;
        other.m_pPool = nullptr;
        other.m_index = 0;
        other.m_count = 1;
    }
    return *this;
}

/// @brief CPUハンドルの取得
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetCPUHandle(
    uint32_t offset) const {
    if (!IsValid()) {
        assert(false && "Invalid DescriptorAllocation");
        return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    }
    if (offset >= m_count) {
        assert(false && "Offset exceeds the allocated range");
        return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    }
    return m_pPool->GetCPUHandle(m_index + offset);
}

/// @brief GPUハンドルの取得
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetGPUHandle(
    uint32_t offset) const {
    if (!IsValid()) {
        assert(false && "Invalid DescriptorAllocation");
        return D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
    }
    if (offset >= m_count) {
        assert(false && "Offset exceeds the allocated range");
        return D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
    }
    return m_pPool->GetGPUHandle(m_index + offset);
}

// 解放
void DescriptorAllocation::Release() {
    if (m_pPool) {
        for (uint32_t i = 0; i < m_count; ++i) {
            m_pPool->Free(m_index + i);
        }
        m_pPool = nullptr;
        m_index = 0;
        m_count = 1;
    }
}