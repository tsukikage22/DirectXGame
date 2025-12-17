#pragma once

#include <d3d12.h>

#include "Engine/GPUBuffer.h"

class VertexBuffer {
public:
    VertexBuffer() : m_Size(0), m_Stride(0) {}

    ~VertexBuffer() { m_Buffer.Term(); }

    // 頂点バッファの初期化
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        size_t size, size_t stride, const void* pInitData = nullptr) {
        m_Size   = size;
        m_Stride = stride;

        if (!m_Buffer.CreateStatic(pDevice, pCmdList, size, pInitData,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)) {
            return false;
        }

        // ビューの設定
        m_View.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        m_View.SizeInBytes    = static_cast<UINT>(m_Size);
        m_View.StrideInBytes  = static_cast<UINT>(m_Stride);

        return true;
    }

    // 頂点バッファの初期化
    template <typename T>
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        size_t size, const T* pInitData = nullptr) {
        return Init(pDevice, pCmdList, size, sizeof(T), pInitData);
    }

    // 頂点バッファの初期化 batch版
    bool Init(ID3D12Device* pDevice, DirectX::ResourceUploadBatch& batch,
        size_t size, size_t stride, const void* pInitData = nullptr) {
        m_Size   = size;
        m_Stride = stride;

        size_t count = size / stride;
        if (!m_Buffer.CreateStatic(pDevice, batch, count, stride, pInitData,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)) {
            return false;
        }

        // ビューの設定
        m_View.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        m_View.SizeInBytes    = static_cast<UINT>(m_Size);
        m_View.StrideInBytes  = static_cast<UINT>(m_Stride);

        return true;
    }

    template <typename T>
    bool Init(ID3D12Device* pDevice, DirectX::ResourceUploadBatch& batch,
        size_t size, const T* pInitData = nullptr) {
        return Init(pDevice, batch, size, sizeof(T), pInitData);
    }

    bool Term() {
        m_Buffer.Term();
        return true;
    }

    void DiscardUpload() { m_Buffer.DiscardUpload(); }

    D3D12_VERTEX_BUFFER_VIEW GetView() { return m_View; }

private:
    GPUBuffer m_Buffer;               // バッファリソース
    D3D12_VERTEX_BUFFER_VIEW m_View;  // 頂点バッファビュー
    size_t m_Size;                    // 頂点バッファのサイズ
    size_t m_Stride;                  // 1頂点あたりのサイズ

    VertexBuffer(const VertexBuffer&)            = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
};
