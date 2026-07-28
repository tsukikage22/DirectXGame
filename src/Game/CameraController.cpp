#include "Game/CameraController.h"

#include <DirectXMath.h>
#include <windows.h>

#include <algorithm>
#include <cmath>

#include "Engine/Scene/Transform.h"

CameraController::CameraController()
    : m_pCamera(nullptr),
      m_pInputSystem(nullptr),
      m_moveSpeed(3.0f),
      m_sensitivity(0.2f),
      m_pitch(0.0f),
      m_yaw(0.0f) {}

CameraController::~CameraController() {}

void CameraController::Init(Camera* pCamera, InputSystem* pInputSystem) {
    m_pCamera      = pCamera;
    m_pInputSystem = pInputSystem;

    // 初期姿勢の取得
    if (m_pCamera) {
        DirectX::XMFLOAT3 forward = m_pCamera->GetTransform().GetForward();
        DirectX::XMFLOAT2 euler   = ForwardToEuler(forward);
        m_pitch                   = euler.x;
        m_yaw                     = euler.y;
    }
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

    // 回転処理
    // マウスの右ボタン押下中に，マウスの移動量に基づいてカメラを回転させる
    // DXはyaw，DYはpitch
    if (m_pInputSystem->IsMouseDown(Button::Right)) {
        int mouseDX = m_pInputSystem->MouseDX();
        int mouseDY = m_pInputSystem->MouseDY();
        m_yaw += m_sensitivity * mouseDX;
        m_pitch += m_sensitivity * mouseDY;
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        m_pCamera->GetTransform().SetRotation(m_pitch, m_yaw, 0.0f);
    }

    // 移動処理（WASD）
    float moveZ = 0.0f;
    float moveX = 0.0f;
    float moveY = 0.0f;
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
    if (m_pInputSystem->IsKeyDown(VK_SPACE)) {
        moveY += m_moveSpeed * deltaTime;
    }
    if (m_pInputSystem->IsKeyDown(VK_SHIFT)) {
        moveY -= m_moveSpeed * deltaTime;
    }

    // カメラの前方向と右方向を取得
    DirectX::XMFLOAT3 forward = m_pCamera->GetTransform().GetForward();
    DirectX::XMFLOAT3 right   = m_pCamera->GetTransform().GetRight();
    pos.x += moveX * right.x + moveZ * forward.x;
    pos.y += moveX * right.y + moveZ * forward.y;
    pos.z += moveX * right.z + moveZ * forward.z;
    pos.y += moveY;
    m_pCamera->GetTransform().SetPosition(pos);
}

// Forwardベクトルからオイラー角への変換
DirectX::XMFLOAT2 CameraController::ForwardToEuler(
    const DirectX::XMFLOAT3& forward) {
    float fy    = std::clamp(forward.y, -1.0f, 1.0f);
    float pitch = DirectX::XMConvertToDegrees(std::asin(-fy));
    pitch       = std::clamp(pitch, -89.0f, 89.0f);
    float yaw   = DirectX::XMConvertToDegrees(std::atan2(forward.x, forward.z));
    return DirectX::XMFLOAT2(pitch, yaw);
}