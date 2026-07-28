#include "Engine/Scene/Transform.h"

using namespace DirectX;

Transform::Transform()
    : m_position{ 0.0f, 0.0f, 0.0f },
      m_scale{ 1.0f, 1.0f, 1.0f },
      m_orientation{ 0.0f, 0.0f, 0.0f, 1.0f } {}

Transform::~Transform() {}

XMMATRIX Transform::CalcWorldMatrix() const {
    // スケーリング
    XMMATRIX scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

    // 回転
    XMVECTOR q              = XMLoadFloat4(&m_orientation);
    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(q);

    // 平行移動
    XMMATRIX translationMatrix =
        XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

    // ワールド行列の合成（S * R * T）
    XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

    return worldMatrix;
}

// オイラー角で姿勢を設定する
void Transform::SetRotation(float pitch, float yaw, float roll) {
    // ラジアンに変換
    pitch = XMConvertToRadians(pitch);
    yaw   = XMConvertToRadians(yaw);
    roll  = XMConvertToRadians(roll);

    // クォータニオンを計算
    XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
    XMStoreFloat4(&m_orientation, quaternion);
}

// ワールド座標系での回転操作
void XM_CALLCONV Transform::RotateWorld(FXMVECTOR axis, float angleDeg) {
    // 回転操作のクオータニオンを計算
    XMVECTOR rotationQuat =
        XMQuaternionRotationAxis(axis, XMConvertToRadians(angleDeg));
    // ワールド座標系での回転なので現在の姿勢を適用してから回転をかける
    XMVECTOR orientation =
        XMQuaternionMultiply(XMLoadFloat4(&m_orientation), rotationQuat);
    XMStoreFloat4(&m_orientation, XMQuaternionNormalize(orientation));
}

// ローカル座標系での回転操作
void XM_CALLCONV Transform::RotateLocal(FXMVECTOR axis, float angleDeg) {
    // 回転操作のクオータニオンを計算
    XMVECTOR rotationQuat =
        XMQuaternionRotationAxis(axis, XMConvertToRadians(angleDeg));
    // ローカル座標系での回転なので回転をかけてから現在の姿勢を適用する
    XMVECTOR orientation =
        XMQuaternionMultiply(rotationQuat, XMLoadFloat4(&m_orientation));
    XMStoreFloat4(&m_orientation, XMQuaternionNormalize(orientation));
}

XMFLOAT3 Transform::GetForward() const {
    // ワールドの基底ベクトルをクオータニオンで回転させる
    XMVECTOR forward = XMVector3Rotate(
        XMLoadFloat3(&engine::kForward), XMLoadFloat4(&m_orientation));
    XMFLOAT3 forwardFloat3;
    XMStoreFloat3(&forwardFloat3, forward);
    return forwardFloat3;
}

XMFLOAT3 Transform::GetUp() const {
    // ワールドの基底ベクトルをクオータニオンで回転させる
    XMVECTOR up = XMVector3Rotate(
        XMLoadFloat3(&engine::kUp), XMLoadFloat4(&m_orientation));
    XMFLOAT3 upFloat3;
    XMStoreFloat3(&upFloat3, up);
    return upFloat3;
}

XMFLOAT3 Transform::GetRight() const {
    // ワールドの基底ベクトルをクオータニオンで回転させる
    XMVECTOR right = XMVector3Rotate(
        XMLoadFloat3(&engine::kRight), XMLoadFloat4(&m_orientation));
    XMFLOAT3 rightFloat3;
    XMStoreFloat3(&rightFloat3, right);
    return rightFloat3;
}