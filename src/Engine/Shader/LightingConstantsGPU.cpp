#include "Engine/Shader/LightingConstantsGPU.h"

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
    lc.lightType                 = 0;           // ディレクショナルライト
    lc.lightForward   = { 0.0f, -1.0f, 0.0f };  // ライトの向き（下向き）
    lc.lightIntensity = 5.0f;                   // 強さ
    lc.lightColor     = { 1.0f, 1.0f, 1.0f };   // 色

    lc.lightAngleScale =  // 角度減衰係数
        1.0f /
        DirectX::XMMax(0.001f, cosf(DirectX::XMConvertToRadians(15.0f)) -
                                   cosf(DirectX::XMConvertToRadians(20.0f)));

    lc.lightAngleOffset =  // 角度オフセット
        -cosf(DirectX::XMConvertToRadians(20.0f)) * lc.lightAngleScale;

    return Init(pDevice, pPoolCBV, lc);
}

void LightingConstantsGPU::Term() { m_constantBuffer.Term(); }

void LightingConstantsGPU::Update(
    const shader::LightingConstants& lightingConstants) {
    m_constants = lightingConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}