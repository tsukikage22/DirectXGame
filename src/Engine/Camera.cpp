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
DirectX::XMMATRIX Camera::GetViewMatrix() const {
    DirectX::XMVECTOR eyePos    = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&m_target);
    DirectX::XMVECTOR upward    = DirectX::XMLoadFloat3(&m_up);
    return DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
}

/// @brief 射影行列の計算
/// @return
DirectX::XMMATRIX Camera::GetProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovRH(m_fovY, m_aspect, m_nearZ, m_farZ);
}
