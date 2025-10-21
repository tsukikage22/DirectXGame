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

void SceneConstantsGPU::Term() { m_constantBuffer.Term(); }

void SceneConstantsGPU::Update(const shader::SceneConstants& sceneConstants) {
    m_constants = sceneConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}