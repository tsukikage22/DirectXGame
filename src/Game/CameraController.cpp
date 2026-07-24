#include "Game/CameraController.h"

#include <DirectXMath.h>

#include "Engine/Scene/Transform.h"

CameraController::CameraController()
    : m_pCamera(nullptr),
      m_pInputSystem(nullptr),
      m_moveSpeed(3.0f),
      m_rotateSpeed(10.0f) {}

CameraController::~CameraController() {}

void CameraController::Init(Camera* pCamera, InputSystem* pInputSystem) {
    m_pCamera      = pCamera;
    m_pInputSystem = pInputSystem;
}

void CameraController::Term() {
    m_pCamera      = nullptr;
    m_pInputSystem = nullptr;
}

void CameraController::Update(float deltaTime) {
    if (!m_pCamera || !m_pInputSystem) {
        return;
    }

    DirectX::XMFLOAT3 pos = m_pCamera->GetTransform().GetPosition();
    float yaw             = 0.0f;

    // yaw回転処理（Q/E）
    if (m_pInputSystem->IsKeyDown('Q')) {
        yaw -= m_rotateSpeed * deltaTime;
    }
    if (m_pInputSystem->IsKeyDown('E')) {
        yaw += m_rotateSpeed * deltaTime;
    }
    if (yaw != 0.0f) {
        m_pCamera->GetTransform().RotateWorld(
            DirectX::XMLoadFloat3(&engine::kUp), yaw);
    }

    // 移動処理（WASD）
    float moveZ = 0.0f;
    float moveX = 0.0f;
    if (m_pInputSystem->IsKeyDown('W')) {
        moveZ += m_moveSpeed * deltaTime;
    }
    if (m_pInputSystem->IsKeyDown('S')) {
        moveZ -= m_moveSpeed * deltaTime;
    }
    if (m_pInputSystem->IsKeyDown('A')) {
        moveX -= m_moveSpeed * deltaTime;
    }
    if (m_pInputSystem->IsKeyDown('D')) {
        moveX += m_moveSpeed * deltaTime;
    }

    // カメラの前方向と右方向を取得
    DirectX::XMFLOAT3 forward = m_pCamera->GetTransform().GetForward();
    DirectX::XMFLOAT3 right   = m_pCamera->GetTransform().GetRight();
    pos.x += moveX * right.x + moveZ * forward.x;
    pos.y += moveX * right.y + moveZ * forward.y;
    pos.z += moveX * right.z + moveZ * forward.z;
    m_pCamera->GetTransform().SetPosition(pos);
}