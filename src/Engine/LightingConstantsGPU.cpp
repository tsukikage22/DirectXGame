#include "Engine/LightingConstantsGPU.h"

LightingConstantsGPU::LightingConstantsGPU()
    : m_constantBuffer(), m_constants() {}

LightingConstantsGPU::~LightingConstantsGPU() { Term(); }

bool LightingConstantsGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    const shader::LightingConstants& lightingConstants) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // 定数バッファの作成
    if (!m_constantBuffer.Init<shader::LightingConstants>(pDevice, pPoolCBV)) {
        return false;
    }

    // 初期値のセット
    Update(lightingConstants);

    return true;
}

bool LightingConstantsGPU::Init(
    ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // デフォルト値の設定
    shader::LightingConstants lc = {};
    lc.lightDirection            = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
    lc.lightIntensity            = 1.0f;
    lc.lightColor                = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    return Init(pDevice, pPoolCBV, lc);
}

void LightingConstantsGPU::Term() { m_constantBuffer.Term(); }

void LightingConstantsGPU::Update(
    const shader::LightingConstants& lightingConstants) {
    m_constants = lightingConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}