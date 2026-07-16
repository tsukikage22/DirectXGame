#pragma once

#include <d3d12.h>

#include <cassert>
#include <memory>
#include <vector>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Graphics/GPUBuffer.h"

class DescriptorPool;

class ConstantBuffer {
public:
    ConstantBuffer();
    ~ConstantBuffer();

    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, size_t size);

    template <typename T>
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool) {
        return Init(pDevice, pPool, sizeof(T));
    };

    void Term();

    void Update(const void* pData, size_t size);

    //========================================
    // アクセサ
    //========================================
    /// @brief ディスクリプタハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const {
        return m_allocation.GetGPUHandle();
    }

    /// @brief GPU仮想アドレスの取得
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
        return m_GPUAddress;
    }

private:
    DescriptorPool* m_pPool;                 // ディスクリプタプール
    DescriptorAllocation m_allocation;       // ディスクリプタの割り当て
    void* m_pMappedData;                     // マップ済みデータ
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;  // GPU仮想アドレス
    GPUBuffer m_buffer;                      // アップロード用バッファ
    size_t m_size;                           // バッファサイズ

    ConstantBuffer(const ConstantBuffer&)            = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;
};
