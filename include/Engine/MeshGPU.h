/// @file MeshGPU.h
/// @brief GPUが扱うメッシュデータ

#pragma once

#include <DirectXTK12/DDSTextureLoader.h>
#include <d3d12.h>

#include <memory>

#include "Engine/IndexBuffer.h"
#include "Engine/ModelAsset.h"
#include "Engine/VertexBuffer.h"

class MeshGPU {
public:
    MeshGPU();
    ~MeshGPU() { Term(); }

    // 初期化処理・VB/IBの作成
    bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
        const MeshAsset& mesh);

    /// @brief VB/IBの作成 batch版
    bool Init(ID3D12Device* pDevice, DirectX::ResourceUploadBatch& batch,
        const MeshAsset& mesh);

    void Term();

    /// @brief アップロード用バッファの破棄
    void DiscardUpload();

    //==============================================================
    // アクセサ
    //==============================================================
    /// @brief 頂点バッファビューのgetter
    /// @return 頂点バッファビュー
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const {
        return m_pVB->GetView();
    }

    /// @brief インデックスバッファビューのgetter
    /// @return インデックスバッファビュー
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const {
        return m_pIB->GetView();
    }

    /// @brief インデックスデータの数のgetter
    /// @return インデックスデータの数
    uint32_t GetIndexCount() const { return m_IndexCount; }

    /// @brief マテリアルIDのgetter
    /// @return マテリアルID
    uint32_t GetMaterialID() const { return m_MaterialID; }

private:
    std::unique_ptr<VertexBuffer> m_pVB;
    std::unique_ptr<IndexBuffer> m_pIB;
    uint32_t m_MaterialID;
    uint32_t m_IndexCount;

    // コピー禁止
    MeshGPU(const MeshGPU&)            = delete;
    MeshGPU& operator=(const MeshGPU&) = delete;
};
