#pragma once

#include <d3d12.h>

#include <cassert>
#include <type_traits>

#include "Engine/GPUBuffer.h"

class IndexBuffer {
public:
    IndexBuffer() : m_View() {}

    ~IndexBuffer() { Term(); }

    // インデックスバッファの初期化
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        size_t size, DXGI_FORMAT format, const void* pInitData = nullptr) {
        // 引数チェック
        if (pDevice == nullptr || pCmdList == nullptr || size == 0) {
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

    template <typename T>
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        const std::vector<T>& indices) {
        // インデックスの型チェック
        static_assert(
            std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>,
            "Index type must be uint16_t or uint32_t.");
        // formatの決定
        DXGI_FORMAT format =
            (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        return Init(pDevice, pCmdList, sizeof(T) * indices.size(), format,
            indices.data());
    }

    // インデックスバッファの初期化 batch版
    bool Init(ID3D12Device* pDevice, DirectX::ResourceUploadBatch& batch,
        size_t size, DXGI_FORMAT format, const void* pInitData = nullptr) {
        // 引数チェック
        if (pDevice == nullptr || size == 0) {
            return false;
        }

        const size_t stride = (format == DXGI_FORMAT_R16_UINT) ? 2 : 4;
        const size_t count  = size / stride;

        // バッファリソースの生成
        if (!m_Buffer.CreateStatic(pDevice, batch, count, stride, pInitData,
                D3D12_RESOURCE_STATE_INDEX_BUFFER)) {
            return false;
        }

        // ビューの設定
        m_View.BufferLocation = m_Buffer.GetGPUVirtualAddress();
        m_View.SizeInBytes    = static_cast<UINT>(size);
        m_View.Format         = format;

        return true;
    }

    template <typename T>
    bool Init(ID3D12Device* pDevice, DirectX::ResourceUploadBatch& batch,
        const std::vector<T>& indices) {
        // インデックスの型チェック
        static_assert(
            std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>,
            "Index type must be uint16_t or uint32_t.");
        // formatの決定
        DXGI_FORMAT format =
            (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        return Init(
            pDevice, batch, sizeof(T) * indices.size(), format, indices.data());
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