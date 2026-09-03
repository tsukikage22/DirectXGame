#include "Engine/Scene/Camera.h"

#include <cassert>
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

// カメラの視錐台を覆う球を計算する
DirectX::BoundingSphere Camera::ComputeBoundingSphere(
    float nearZ, float farZ) const {
    assert(farZ > nearZ && "farZ must be greater than nearZ");

    // 視錐台の広がり（far面の外接円の半径が farZ * sqrt(k2)になる）
    float t  = std::tan(m_fovYRad * 0.5f);
    float k2 = t * t * (m_aspect * m_aspect + 1.0f);

    // near面とfar面の四隅から等距離になる点をビュー空間で求める
    float z = (farZ + nearZ) * (k2 + 1.0f) * 0.5f;
    float r = 0.0f;

    // 球の半径
    if (z >= farZ) {
        // 球の中心がfar面よりも遠くにある場合は，球の半径はfar面の外接円の半径になる
        z = farZ;
        r = farZ * std::sqrt(k2);
    } else {
        // 球の中心がfar面よりも近い場合は，球の半径は球の中心からfar面の四隅までの距離になる
        r = std::sqrt(k2 * farZ * farZ + (farZ - z) * (farZ - z));
    }

    // ワールド座標での球の中心
    XMFLOAT3 worldPos     = m_transform.GetPosition();
    XMFLOAT3 worldForward = m_transform.GetForward();
    XMFLOAT3 center       = {
        worldPos.x + worldForward.x * z,
        worldPos.y + worldForward.y * z,
        worldPos.z + worldForward.z * z,
    };

    return DirectX::BoundingSphere(center, r);
}
