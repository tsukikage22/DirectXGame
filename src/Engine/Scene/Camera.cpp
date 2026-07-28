#include "Engine/Scene/Camera.h"

using namespace DirectX;

Camera::Camera()
    : m_fovYRad(XMConvertToRadians(45.0f)),
      m_aspect(16.0f / 9.0f),
      m_nearZ(1.0f),
      m_farZ(1000.0f) {
    // 初期位置を設定
    m_transform.SetPosition({ 0.0f, 0.0f, -5.0f });
}

/// @brief ビュー行列の計算
/// @return
XMFLOAT4X4 Camera::GetViewMatrix() {
    // ビュー行列はカメラのワールド行列の逆行列
    // また，スケールがないため剛体変換なので
    // 逆行列は回転行列の転置と並進の反転で計算できる
    XMFLOAT4 q          = m_transform.GetOrientation();
    XMMATRIX R          = XMMatrixRotationQuaternion(XMLoadFloat4(&q));
    XMFLOAT3 pos        = m_transform.GetPosition();
    XMVECTOR posVec     = XMLoadFloat3(&pos);
    XMMATRIX viewMatrix = XMMatrixTranspose(R);
    viewMatrix.r[3] =
        XMVectorSetW(-XMVector3Transform(posVec, viewMatrix), 1.0f);
    XMStoreFloat4x4(&m_viewMatrix, viewMatrix);
    return m_viewMatrix;
}

/// @brief 射影行列の計算
/// @return
XMFLOAT4X4 Camera::GetProjectionMatrix() {
    DirectX::XMMATRIX projMatrix =
        DirectX::XMMatrixPerspectiveFovLH(m_fovYRad, m_aspect, m_nearZ, m_farZ);
    DirectX::XMStoreFloat4x4(&m_projMatrix, projMatrix);
    return m_projMatrix;
}