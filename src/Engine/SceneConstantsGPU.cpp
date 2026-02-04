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
    if (!m_constantBuffer.Init<shader::SceneConstants>(pDevice, pPoolCBV)) {
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
        DirectX::XMMatrixLookAtLH(eyePos, targetPos, upward);
    DirectX::XMMATRIX viewT = DirectX::XMMatrixTranspose(viewMatrix);
    DirectX::XMStoreFloat4x4(&sc.view, viewT);

    // projection行列の初期化
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovRH(
        DirectX::XMConvertToRadians(45.0f), 16.0f / 9.0f, 1.0f, 1000.0f);
    DirectX::XMMATRIX projT = DirectX::XMMatrixTranspose(projMatrix);
    DirectX::XMStoreFloat4x4(&sc.projection, projT);

    // カメラ位置・ゲーム時間・露出の初期化
    sc.cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
    sc.time           = 0.0f;
    sc.exposure       = 1.0f;

    return Init(pDevice, pPoolCBV, sc);
}

void SceneConstantsGPU::Term() { m_constantBuffer.Term(); }

void SceneConstantsGPU::Update(const shader::SceneConstants& sceneConstants) {
    m_constants = sceneConstants;
    m_constantBuffer.Update(&m_constants, sizeof(m_constants));
}