#pragma once

#include <d3d12.h>

#include <cstring>

#include "Engine/ComPtr.h"

class GPUBuffer {
public:
    GPUBuffer() : m_Size(0) {}
    ~GPUBuffer() { Term(); }

    bool CreateStatic(ID3D12Device* pDevice,
        ID3D12GraphicsCommandList* pCmdList, size_t size, const void* pInitData,
        D3D12_RESOURCE_STATES finalState);

    void DiscardUpload();

    void Term();

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

private:
    engine::ComPtr<ID3D12Resource> m_pRes;     // バッファリソース
    engine::ComPtr<ID3D12Resource> m_pUpload;  // アップロード用バッファリソース
    size_t m_Size;
    D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATE_COMMON;
};
