/// @file DescriptorAllocation.h
/// @brief ディスクリプタプールのスロットを所有し，破棄時に解放する

#pragma once

#include <d3d12.h>

#include <cassert>
#include <cstdint>

// 前方宣言
class DescriptorPool;

class DescriptorAllocation {
public:
    // デフォルトコンストラクタ
    DescriptorAllocation() = default;

    // コンストラクタ
    DescriptorAllocation(DescriptorPool* pPool, uint32_t index);
    DescriptorAllocation(DescriptorPool* pPool, uint32_t index, uint32_t count);

    // デストラクタ
    ~DescriptorAllocation();

    // コピー禁止（デストラクタでFreeするので唯一の所有権である必要がある）
    DescriptorAllocation(const DescriptorAllocation&)            = delete;
    DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

    // ムーブ
    DescriptorAllocation(DescriptorAllocation&&) noexcept;
    DescriptorAllocation& operator=(DescriptorAllocation&&) noexcept;

    /// @brief インデックスの取得
    uint32_t GetIndex() const { return m_index; }

    /// @brief 連続数の取得
    uint32_t GetCount() const { return m_count; }

    /// @brief CPUハンドルの取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t offset = 0) const;

    /// @brief GPUハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t offset = 0) const;

    /// @brief 有効かどうかの確認
    bool IsValid() const { return m_pPool != nullptr && m_index != UINT32_MAX; }

private:
    void Release();

    DescriptorPool* m_pPool = nullptr;  // 所属するディスクリプタプール
    uint32_t m_index        = 0;        // ディスクリプタのインデックス
    uint32_t m_count        = 1;  // 連続数（連続領域を確保した場合は1以上）
};