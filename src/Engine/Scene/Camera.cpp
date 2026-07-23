#include "Engine/Scene/Camera.h"

Camera::Camera()
    : m_position(0.0f, 0.0f, -5.0f),
      m_orientation(0.0f, 0.0f, 0.0f, 1.0f),
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

    // rotaionをラジアンに変換
    DirectX::XMFLOAT3 rotationRad = { DirectX::XMConvertToRadians(m_rotation.x),
        DirectX::XMConvertToRadians(m_rotation.y),
        DirectX::XMConvertToRadians(m_rotation.z) };

    // 回転行列を作成
    DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        rotationRad.x, rotationRad.y, rotationRad.z);

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

    // ラジアンをオイラー角に変換
    m_rotation.x = DirectX::XMConvertToDegrees(m_rotation.x);
    m_rotation.y = DirectX::XMConvertToDegrees(m_rotation.y);
}

// forwardの計算
DirectX::XMFLOAT3 Camera::GetForward() const {
    DirectX::XMVECTOR q       = DirectX::XMLoadFloat4(&m_orientation);
    DirectX::XMVECTOR forward = DirectX::XMVectorSet(0, 0, 1, 0);
    forward                   = DirectX::XMVector3Rotate(forward, q);
    DirectX::XMFLOAT3 forwardFloat3;
    DirectX::XMStoreFloat3(&forwardFloat3, forward);
    return forwardFloat3;
}

// upの計算
DirectX::XMFLOAT3 Camera::GetUp() const {
    DirectX::XMVECTOR q  = DirectX::XMLoadFloat4(&m_orientation);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
    up                   = DirectX::XMVector3Rotate(up, q);
    DirectX::XMFLOAT3 upFloat3;
    DirectX::XMStoreFloat3(&upFloat3, up);
    return upFloat3;
}

// rightの計算
DirectX::XMFLOAT3 Camera::GetRight() const {
    DirectX::XMVECTOR q     = DirectX::XMLoadFloat4(&m_orientation);
    DirectX::XMVECTOR right = DirectX::XMVectorSet(1, 0, 0, 0);
    right                   = DirectX::XMVector3Rotate(right, q);
    DirectX::XMFLOAT3 rightFloat3;
    DirectX::XMStoreFloat3(&rightFloat3, right);
    return rightFloat3;
}
