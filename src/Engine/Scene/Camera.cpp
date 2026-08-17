#include "Engine/Scene/Camera.h"

#include <cmath>

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

// 露出パラメータの設定
void Camera::SetExposure(float aperture, float shutterSpeed, float iso) {
    m_aperture     = aperture;
    m_shutterSpeed = shutterSpeed;
    m_iso          = iso;
}

// EV100の計算
float Camera::ComputeEV100() const {
    // EV100 = log2((apeature^2) / shutterSpeed * 100 / iso)
    return std::log2(
        (m_aperture * m_aperture) / m_shutterSpeed * 100.0f / m_iso);
}

// 露出の計算
float Camera::ComputeExposure() const {
    float ev100    = ComputeEV100();
    float exposure = 1.0f / (1.2f * std::pow(2.0f, ev100));
    return exposure;
}

// シャッタースピードを固定し，EV100を指定して絞り値を計算する
float Camera::ComputeAperture(float ev100) const {
    // aperture = sqrt(shutterSpeed * 2^EV100 * iso / 100)
    return std::sqrt(m_shutterSpeed * std::pow(2.0f, ev100) * m_iso / 100.0f);
}

// 絞り値を固定し，EV100を指定してシャッタースピードを計算する
float Camera::ComputeShutterSpeed(float ev100) const {
    // shutterSpeed = (aperture^2) / (2^EV100 * iso / 100)
    return (m_aperture * m_aperture) / (std::pow(2.0f, ev100) * m_iso / 100.0f);
}

// EV100を指定して露出パラメータを更新する
void Camera::ApplyEV100(float ev100, bool fixShutterSpeed) {
    if (fixShutterSpeed) {
        m_aperture = ComputeAperture(ev100);

    } else {
        m_shutterSpeed = ComputeShutterSpeed(ev100);
    }
}