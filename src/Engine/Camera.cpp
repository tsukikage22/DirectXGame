#include "Engine/Camera.h"

Camera::Camera()
    : m_position(0.0f, 0.0f, -5.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_target(0.0f, 0.0f, 0.0f),
      m_up(0.0f, 1.0f, 0.0f),
      m_fovY(DirectX::XMConvertToRadians(45.0f)),
      m_aspect(16.0f / 9.0f),
      m_nearZ(1.0f),
      m_farZ(1000.0f) {}

/// @brief ビュー行列の計算
/// @return
DirectX::XMFLOAT4X4 Camera::GetViewMatrix() {
    DirectX::XMVECTOR eyePos    = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&m_target);
    DirectX::XMVECTOR upward    = DirectX::XMLoadFloat3(&m_up);
    DirectX::XMMATRIX viewMatrix =
        DirectX::XMMatrixLookAtLH(eyePos, targetPos, upward);
    DirectX::XMStoreFloat4x4(&m_viewMatrix, viewMatrix);
    return m_viewMatrix;
}

/// @brief 射影行列の計算
/// @return
DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix() {
    DirectX::XMMATRIX projMatrix =
        DirectX::XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
    DirectX::XMStoreFloat4x4(&m_projMatrix, projMatrix);
    return m_projMatrix;
}
