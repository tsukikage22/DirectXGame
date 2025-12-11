/// @file MeshGPU.cpp
/// @brief GPUが扱うメッシュデータ

#include "Engine/MeshGPU.h"

// コンストラクタ
MeshGPU::MeshGPU()
    : m_pVB(nullptr),
      m_pIB(nullptr),
      m_MaterialID(UINT32_MAX),
      m_IndexCount(0) {}

// 初期化処理・VB/IBの作成
bool MeshGPU::Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
    const MeshAsset& mesh) {
    // 引数チェック
    if (!pDevice) {
        return false;
    }

    // 頂点バッファの作成
    auto pVB = std::make_unique<VertexBuffer>();
    if (!pVB->Init(pDevice, pCmdList,
            sizeof(StandardVertex) * mesh.vertices.size(),
            mesh.vertices.data())) {
        return false;
    }
    m_pVB = std::move(pVB);

    // インデックスバッファの作成
    auto pIB = std::make_unique<IndexBuffer>();
    if (!pIB->Init(pDevice, pCmdList, mesh.indices)) {
        return false;
    }
    m_pIB = std::move(pIB);

    m_MaterialID = mesh.materialID;
    m_IndexCount = static_cast<uint32_t>(mesh.indices.size());

    return true;
}

// 終了処理，リソースの解放
void MeshGPU::Term() {
    m_pVB->Term();
    m_pIB->Term();
    m_pVB.reset();
    m_pIB.reset();
}

void MeshGPU::DiscardUpload() {
    if (m_pVB) {
        m_pVB->DiscardUpload();
    }
    if (m_pIB) {
        m_pIB->DiscardUpload();
    }
}
