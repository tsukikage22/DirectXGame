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
    DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&m_position);

    // rotationから回転行列を作成
    DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        m_rotation.x, m_rotation.y, m_rotation.z);

    // 前方ベクトルと上方向ベクトルを回転させる
    DirectX::XMVECTOR forward = DirectX::XMVector3TransformNormal(
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
    DirectX::XMVECTOR up = DirectX::XMVector3TransformNormal(
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);

    // 注視点を計算
    DirectX::XMVECTOR target = DirectX::XMVectorAdd(eyePos, forward);
    DirectX::XMStoreFloat3(&m_target, target);
    DirectX::XMStoreFloat3(&m_up, up);

    // ビュー行列を計算
    DirectX::XMMATRIX viewMatrix =
        DirectX::XMMatrixLookAtLH(eyePos, target, up);

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

void Camera::SetTarget(const DirectX::XMFLOAT3& target) {
    m_target = target;

    // targetへのベクトルを計算
    float dx = m_target.x - m_position.x;
    float dy = m_target.y - m_position.y;
    float dz = m_target.z - m_position.z;

    // 水平距離
    float xzDist = sqrt(dx * dx + dz * dz);

    // 回転角度を計算（ラジアン）
    m_rotation.y = atan2(dx, dz);       // yaw
    m_rotation.x = -atan2(dy, xzDist);  // pitch
    m_rotation.z = 0.0f;                // roll
}