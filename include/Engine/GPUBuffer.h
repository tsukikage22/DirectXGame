#pragma once

#include <d3d12.h>
#include <directXTK12/ResourceUploadBatch.h>
#include <directxtk12/BufferHelpers.h>

#include <cstring>
#include <optional>

#include "Engine/ComPtr.h"

class GPUBuffer {
public:
    GPUBuffer() : m_Size(0) {}
    ~GPUBuffer() { Term(); }

    /// @brief VBやIBなどの静的バッファを作成する，コマンドリスト使用
    bool CreateStatic(ID3D12Device* pDevice,
        ID3D12GraphicsCommandList* pCmdList, size_t size, const void* pInitData,
        D3D12_RESOURCE_STATES finalState);

    /// @brief VBやIBなどの静的バッファを作成する，ResourceUploadBatch使用
    bool CreateStatic(ID3D12Device* pDevice,
        DirectX::ResourceUploadBatch& batch, size_t count, size_t stride,
        const void* pInitData, D3D12_RESOURCE_STATES finalState);

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
