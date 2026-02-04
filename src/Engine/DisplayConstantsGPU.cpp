#include "Engine/DisplayConstantsGPU.h"

DisplayConstantsGPU::DisplayConstantsGPU() : m_constantBuffer() {}

DisplayConstantsGPU::~DisplayConstantsGPU() { Term(); }

bool DisplayConstantsGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    const shader::DisplayConstants& displayConstants) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // 定数バッファの作成
    if (!m_constantBuffer.Init<shader::DisplayConstants>(pDevice, pPoolCBV)) {
        return false;
    }

    // 初期値のセット
    Update(displayConstants);

    return true;
}

bool DisplayConstantsGPU::Init(
    ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    shader::DisplayConstants dc = {};

    // デフォルト値の設定
    dc.maxLuminance          = 80.0f;  // 最大輝度
    dc.minLuminance          = 0.0f;   // 最小
    dc.paperWhiteNits        = 80.0f;  // SDRの白の明るさ
    dc.maxFullFrameLuminance = 80.0f;  // 全白時の最大輝度

    return Init(pDevice, pPoolCBV, dc);
}

void DisplayConstantsGPU::Term() { m_constantBuffer.Term(); }

void DisplayConstantsGPU::Update(
    const shader::DisplayConstants& displayConstants) {
    m_constants = displayConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}