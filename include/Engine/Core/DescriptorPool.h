/////////////////////////////////////////////
/// @file DescriptorPool.h
/// @brief ディスクリプタプール
/////////////////////////////////////////////
#pragma once

#include <d3d12.h>
#include <directxtk12/DescriptorHeap.h>

#include <algorithm>
#include <cassert>
#include <mutex>
#include <vector>

#include "Engine/Core/ComPtr.h"

// 前方宣言
class DescriptorAllocation;

class DescriptorPool {
    // FreeをDescriptorAllocationから呼び出すため
    friend class DescriptorAllocation;

public:
    /////////////////////////////////////////////////////////////////////////////
    /// @brief コンストラクタ
    /////////////////////////////////////////////////////////////////////////////
    DescriptorPool();

    /////////////////////////////////////////////////////////////////////////////
    /// @brief デストラクタ
    /////////////////////////////////////////////////////////////////////////////
    ~DescriptorPool();

    /////////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタヒープの生成
    /// @param[in] pDevice デバイス
    /// @param[in] desc ディスクリプタヒープの設定
    /// @param[out] outPool プールの格納先
    /// @retval true 成功
    /// @retval false 失敗
    /////////////////////////////////////////////////////////////////////////////
    static bool Create(ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags, uint32_t capacity,
        DescriptorPool** outPool);

    /////////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタプールへの割り当て
    /////////////////////////////////////////////////////////////////////////////
    DescriptorAllocation Allocate();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタプールに連続した領域を確保
    /// @param count
    ///////////////////////////////////////////////////////////////////////////
    DescriptorAllocation AllocateRange(uint32_t count);

    //========================================================================
    // アクセサ
    //========================================================================
    // ヒープの取得
    ID3D12DescriptorHeap* GetHeap() const { return m_pHeap->Heap(); }

private:
    //========================================================================
    // private methods
    //========================================================================
    /// @brief インデックスの確保
    uint32_t ReserveIndex();

    /// @brief 割り当ての解放
    void Free(uint32_t index);

    // ハンドルの取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

    //========================================================================
    // private variables
    //========================================================================
    std::unique_ptr<DirectX::DescriptorHeap> m_pHeap;  // ディスクリプタヒープ
    D3D12_DESCRIPTOR_HEAP_TYPE m_type{};  // ディスクリプタヒープの種類
    uint32_t m_capacity;                  // プールのサイズ
    bool m_shaderVisible;
    std::vector<uint32_t> m_free;  // 空きスロット

    // 排他制御のためのmutex
    mutable std::mutex m_mutex;

    // コピー禁止
    DescriptorPool(const DescriptorPool&)            = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
};