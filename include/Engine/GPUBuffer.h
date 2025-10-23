#pragma once

#include <d3d12.h>

#include <cstring>
#include <optional>

#include "Engine/ComPtr.h"

class GPUBuffer {
public:
    GPUBuffer() : m_Size(0) {}
    ~GPUBuffer() { Term(); }

    /// @brief VBやIBなどの静的バッファを作成する
    /// @param pDevice
    /// @param pCmdList
    /// @param size
    /// @param pInitData
    /// @param finalState
    /// @return
    bool CreateStatic(ID3D12Device* pDevice,
        ID3D12GraphicsCommandList* pCmdList, size_t size, const void* pInitData,
        D3D12_RESOURCE_STATES finalState);

    /// @brief 変換行列などの更新が必要なバッファを作成する
    /// @param pDevice
    /// @param size 必要があればアラインメントして渡す
    /// @return
    bool CreateDynamic(ID3D12Device* pDevice, size_t size);

    void DiscardUpload();

    void Term();

    //==============================================================
    // アクセサ
    //==============================================================
    ID3D12Resource* GetResource() const { return m_pRes.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
    void* GetMappedPtr() const { return m_pMappedData; }

private:
    engine::ComPtr<ID3D12Resource> m_pRes;     // バッファリソース
    engine::ComPtr<ID3D12Resource> m_pUpload;  // アップロード用バッファリソース
    size_t m_Size;
    D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATE_COMMON;
    bool m_IsMapped               = false;
    void* m_pMappedData           = nullptr;
};
