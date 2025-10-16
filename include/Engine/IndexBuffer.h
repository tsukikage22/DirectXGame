#pragma once

#include <d3d12.h>

#include "Engine/GPUBuffer.h"

class IndexBuffer {
public:
    IndexBuffer() : m_View() {}

    ~IndexBuffer() { Term(); }

    // インデックスバッファの初期化
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        size_t size, DXGI_FORMAT format, const void* pInitData = nullptr) {
        // 引数チェック
        if (pDevice == nullptr || pCmdList == nullptr || size == 0 ||
            (format != DXGI_FORMAT_R16_UINT &&
                format != DXGI_FORMAT_R32_UINT)) {
            return false;
        }

        // バッファリソースの生成
        if (!m_Buffer.CreateStatic(pDevice, pCmdList, size, pInitData,
                D3D12_RESOURCE_STATE_INDEX_BUFFER)) {
            return false;
        }

        // ビューの設定
        m_View.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        m_View.SizeInBytes    = static_cast<UINT>(size);
        m_View.Format         = format;

        return true;
    }

    bool Term() {
        m_Buffer.Term();
        return true;
    }

    void DiscardUpload() { m_Buffer.DiscardUpload(); }

    // インデックスバッファビューの取得
    const D3D12_INDEX_BUFFER_VIEW GetView() const { return m_View; }

private:
    GPUBuffer m_Buffer;                   // バッファリソース
    D3D12_INDEX_BUFFER_VIEW m_View = {};  // インデックスバッファビュー

    // コピー禁止
    IndexBuffer(const IndexBuffer&)            = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
};