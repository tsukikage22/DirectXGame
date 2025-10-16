/// @file MeshGPU.cpp
/// @brief GPUが扱うメッシュデータ

#include "Engine/MeshGPU.h"

// コンストラクタ
MeshGPU::MeshGPU()
    : m_pVB(nullptr),
      m_pIB(nullptr),
      m_MaterialID(UINT32_MAX),
      m_IndexCount(0) {}

// デストラクタ
MeshGPU::~MeshGPU() { Term(); }

// 初期化処理・VB/IBの作成
bool MeshGPU::Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
    const MeshAsset& mesh) {
    // 引数チェック
    if (!pDevice) {
        return false;
    }

    // 頂点バッファの作成
    VertexBuffer VB;
    if (!VB.Init(pDevice, pCmdList,
            sizeof(StandardVertex) * mesh.vertices.size(),
            mesh.vertices.data())) {
        return false;
    }
    m_pVB = std::make_unique<VertexBuffer>(std::move(VB));

    // インデックスバッファの作成
    IndexBuffer IB;
    if (!IB.Init(pDevice, pCmdList, sizeof(uint32_t) * mesh.indices.size(),
            DXGI_FORMAT_R32_UINT, mesh.indices.data())) {
        return false;
    }
    m_pIB = std::make_unique<IndexBuffer>(std::move(IB));

    m_MaterialID = mesh.materialID;
    m_IndexCount = static_cast<uint32_t>(mesh.indices.size());

    return true;
}

// 終了処理，リソースの解放
void MeshGPU::Term() {
    m_pVB.reset();
    m_pIB.reset();
}
