/////////////////////////////////////////////
/// @file DescriptorPool.h
/// @brief ディスクリプタプール
/////////////////////////////////////////////
#pragma once

#include <d3d12.h>

#include <cassert>
#include <mutex>
#include <vector>

#include "DescriptorHeap.h"
#include "Engine/ComPtr.h"

class DescriptorPool {
public:
    /////////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタヒープの生成
    /// @param[in] pDevice デバイス
    /// @param[in] desc ディスクリプタヒープの設定
    /// @param[out] outPool プールの格納先
    /// @retval true 成功
    /// @retval false 失敗
    /////////////////////////////////////////////////////////////////////////////
    static bool Create(ID3D12Device *pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags, uint32_t capacity,
        DescriptorPool **outPool);

    //////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタプールへの割り当て
    /// @return インデックス
    //////////////////////////////////////////////////////////////////////////
    uint32_t Allocate();

    ///////////////////////////////////////////////////////////////////////////
    /// @brief ディスクリプタプールの解放
    /// @param index インデックス
    ///////////////////////////////////////////////////////////////////////////
    void Free(uint32_t index);

    // ハンドルの取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

    uint32_t Capacity() const;
    uint32_t FreeCount() const;

    /////////////////////////////////////////////////////////////////////////////
    /// @brief コンストラクタ
    /////////////////////////////////////////////////////////////////////////////
    DescriptorPool();

    /////////////////////////////////////////////////////////////////////////////
    /// @brief デストラクタ
    /////////////////////////////////////////////////////////////////////////////
    ~DescriptorPool();

private:
    std::unique_ptr<DirectX::DescriptorHeap> m_pHeap;  // ディスクリプタヒープ
    D3D12_DESCRIPTOR_HEAP_TYPE m_type{};  // ディスクリプタヒープの種類
    uint32_t m_capacity;                  // プールのサイズ
    bool m_shaderVisible;
    std::vector<uint32_t> m_free;  // 空きスロット
    mutable std::mutex m_mutex;

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool &operator=(const DescriptorPool &) = delete;
};