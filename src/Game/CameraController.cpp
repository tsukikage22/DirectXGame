#include "Game/CameraController.h"

CameraController::CameraController()
    : m_pCamera(nullptr),
      m_pInputSystem(nullptr),
      m_moveSpeed(0.1f),
      m_rotateSpeed(0.01f) {}

CameraController::~CameraController() {}

void CameraController::Init(Camera* pCamera, InputSystem* pInputSystem) {
    m_pCamera      = pCamera;
    m_pInputSystem = pInputSystem;
}

void CameraController::Term() {
    m_pCamera      = nullptr;
    m_pInputSystem = nullptr;
}

void CameraController::Update() {
    if (!m_pCamera || !m_pInputSystem) {
        return;
    }

    DirectX::XMFLOAT3 pos = m_pCamera->GetPosition();
    DirectX::XMFLOAT3 rot = m_pCamera->GetRotation();

    // 回転処理（Q/E）
    if (m_pInputSystem->IsKeyDown('Q')) {
        rot.y -= m_rotateSpeed;
    }
    if (m_pInputSystem->IsKeyDown('E')) {
        rot.y += m_rotateSpeed;
    }
    m_pCamera->SetRotation(rot);

    // 移動処理（WASD）
    float moveZ = 0.0f;
    float moveX = 0.0f;
    if (m_pInputSystem->IsKeyDown('W')) {
        moveZ += m_moveSpeed;
    }
    if (m_pInputSystem->IsKeyDown('S')) {
        moveZ -= m_moveSpeed;
    }
    if (m_pInputSystem->IsKeyDown('A')) {
        moveX -= m_moveSpeed;
    }
    if (m_pInputSystem->IsKeyDown('D')) {
        moveX += m_moveSpeed;
    }

    // 簡易的に加算（回転と連動しない）
    pos.x += moveX;
    pos.z += moveZ;
    m_pCamera->SetPosition(pos);
}