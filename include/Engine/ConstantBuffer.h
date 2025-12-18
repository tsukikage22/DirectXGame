#pragma once

#include <d3d12.h>

#include <cassert>
#include <memory>
#include <vector>

#include "Engine/ComPtr.h"
#include "Engine/DescriptorPool.h"
#include "Engine/GPUBuffer.h"

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
        return m_pPool->GetGPUHandle(m_index);
    }

    /// @brief GPU仮想アドレスの取得
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
        return m_GPUAddress;
    }

private:
    DescriptorPool* m_pPool;                 // ディスクリプタプール
    void* m_pMappedData;                     // マップ済みデータ
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;  // GPU仮想アドレス
    uint32_t m_index;    // ディスクリプタプールのインデックス
    GPUBuffer m_buffer;  // アップロード用バッファ
    size_t m_size;       // バッファサイズ

    ConstantBuffer(const ConstantBuffer&)            = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;
};
