#pragma once

#include <d3d12.h>

#include "Engine/GPUBuffer.h"

class VertexBuffer {
public:
    VertexBuffer(size_t size, size_t stride)
        : m_View(), m_Size(size), m_Stride(stride) {}

    bool CreateVB(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        const void* pInitData) {
        return m_Buffer.CreateStatic(pDevice, pCmdList, m_Size, pInitData,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    void DiscardUpload() { m_Buffer.DiscardUpload(); }

    D3D12_VERTEX_BUFFER_VIEW View() const {
        // ビューの設定
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(m_Size);
        view.StrideInBytes = static_cast<UINT>(m_Stride);

        return view;
    }

private:
    GPUBuffer m_Buffer;               // バッファリソース
    D3D12_VERTEX_BUFFER_VIEW m_View;  // 頂点バッファビュー
    size_t m_Size;                    // 頂点バッファのサイズ
    size_t m_Stride;                  // 1頂点あたりのサイズ

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
};
