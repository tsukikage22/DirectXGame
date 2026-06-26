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
    lc.lightType                 = 3;             // フォトメトリックライト
    lc.lightPosition = { 0.0f, 2.25f, -0.625f };  // ライトの位置（上方）

    auto forward = DirectX::XMVector3Normalize(
        DirectX::XMVectorSet(0.0f, -1.0f, 0.285f, 0.0f));
    DirectX::XMStoreFloat3(&lc.lightForward, forward);  // ライトの方向（下方）

    lc.luminousFlux = 100.0f;                // 光束
    lc.lightColor   = { 1.0f, 1.0f, 1.0f };  // 色

    lc.lightAngleScale =  // 角度減衰係数
        1.0f /
        DirectX::XMMax(1e-6f, cosf(DirectX::XMConvertToRadians(15.0f)) -
                                  cosf(DirectX::XMConvertToRadians(45.0f)));

    lc.lightAngleOffset =  // 角度オフセット
        -cosf(DirectX::XMConvertToRadians(45.0f)) * lc.lightAngleScale;

    return Init(pDevice, pPoolCBV, lc);
}

void LightingConstantsGPU::Term() { m_constantBuffer.Term(); }

void LightingConstantsGPU::Update(
    const shader::LightingConstants& lightingConstants) {
    m_constants = lightingConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}