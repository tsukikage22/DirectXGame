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

    // view行列の初期化
    DirectX::XMVECTOR eyePos    = { 0.0f, 0.0f, -5.0f };
    DirectX::XMVECTOR targetPos = { 0.0f, 0.0f, 0.0f };
    DirectX::XMVECTOR upward    = { 0.0f, 1.0f, 0.0f };
    DirectX::XMMATRIX viewMatrix =
        DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
    DirectX::XMStoreFloat4x4(&sc.view, viewMatrix);

    // projection行列の初期化
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovRH(
        DirectX::XMConvertToRadians(45.0f), 16.0f / 9.0f, 1.0f, 1000.0f);
    DirectX::XMStoreFloat4x4(&sc.projection, projMatrix);

    // カメラ位置・ゲーム時間の初期化
    sc.cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
    sc.time           = 0.0f;

    return Init(pDevice, pPoolCBV, sc);
}

void SceneConstantsGPU::Term() { m_constantBuffer.Term(); }

void SceneConstantsGPU::Update(const shader::SceneConstants& sceneConstants) {
    m_constants = sceneConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}