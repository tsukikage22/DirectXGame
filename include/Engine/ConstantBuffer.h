#pragma once

#include <d3d12.h>

#include <cassert>
#include <memory>
#include <vector>

#include "Engine/ComPtr.h"
#include "Engine/GPUBuffer.h"

class DescriptorPool;

class ConstantBuffer {
public:
    ConstantBuffer();
    ~ConstantBuffer();

    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, size_t size);

    void Term();

    void Update(const void* pData, size_t size);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const {
        return m_pPool->GetGPUHandle(m_index);
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
