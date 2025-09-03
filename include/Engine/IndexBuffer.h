#pragma once

#include <d3d12.h>

#include "Engine/GPUBuffer.h"

class IndexBuffer {
public:
    IndexBuffer(size_t size, DXGI_FORMAT format)
        : m_View(), m_Size(size), m_format(format) {}

    bool CreateIB(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        const void* pInitData) {
        return m_Buffer.CreateStatic(pDevice, pCmdList, m_Size, pInitData,
            D3D12_RESOURCE_STATE_INDEX_BUFFER);
    }

    void DiscardUpload() { m_Buffer.DiscardUpload(); }

    D3D12_INDEX_BUFFER_VIEW View() const {
        // ビューの設定
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(m_Size);
        view.Format = m_format;

        return view;
    }

private:
    GPUBuffer m_Buffer;              // バッファリソース
    D3D12_INDEX_BUFFER_VIEW m_View;  // インデックスバッファビュー
    size_t m_Size;                   // インデックスバッファのサイズ
    DXGI_FORMAT m_format;            // インデックスのフォーマット

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
};