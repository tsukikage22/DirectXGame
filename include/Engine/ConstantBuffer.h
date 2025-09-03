#pragma once

#include <d3d12.h>

#include <vector>

#include "Engine/ComPtr.h"

class DescriptorPool;

class ConstantBuffer {
public:
    ConstantBuffer();
    ~ConstantBuffer();

    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, size_t size);

    void Term();

private:
    engine::ComPtr<ID3D12Resource> m_pCB;    // 定数バッファ
    DescriptorPool* m_pPool;                 // ディスクリプタプール
    void* m_pMappedData;                     // マップ済みデータ
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;  // GPU仮想アドレス
    uint32_t m_index;  // ディスクリプタプールのインデックス

    ConstantBuffer(const ConstantBuffer&) = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;
};
