#include "Engine/SceneConstantsGPU.h"

SceneConstantsGPU::SceneConstantsGPU() : m_constantBuffer(), m_constants() {}

SceneConstantsGPU::~SceneConstantsGPU() { Term(); }

bool SceneConstantsGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    const shader::SceneConstants& sceneConstants) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // 定数バッファの作成
    if (!m_constantBuffer.Init(
            pDevice, pPoolCBV, sizeof(shader::SceneConstants))) {
        return false;
    }

    // 初期値のセット
    Update(sceneConstants);

    return true;
}

bool SceneConstantsGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    shader::SceneConstants sc = {};
    DirectX::XMStoreFloat4x4(&sc.view, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(&sc.projection, DirectX::XMMatrixIdentity());
    sc.cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    sc.time           = 0.0f;
    return Init(pDevice, pPoolCBV, sc);
}

void SceneConstantsGPU::Term() { m_constantBuffer.Term(); }

void SceneConstantsGPU::Update(const shader::SceneConstants& sceneConstants) {
    m_constants = sceneConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}