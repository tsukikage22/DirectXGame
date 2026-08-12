#include "Engine/Scene/Transform.h"

#include <cassert>

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
void Transform::RotateWorld(const XMFLOAT3& axis, float angleDeg) {
    XMVECTOR axisVec = XMLoadFloat3(&axis);

    // 回転操作のクオータニオンを計算
    XMVECTOR rotationQuat =
        XMQuaternionRotationAxis(axisVec, XMConvertToRadians(angleDeg));
    // ワールド座標系での回転なので現在の姿勢を適用してから回転をかける
    XMVECTOR orientation =
        XMQuaternionMultiply(XMLoadFloat4(&m_orientation), rotationQuat);
    XMStoreFloat4(&m_orientation, XMQuaternionNormalize(orientation));
}

// ローカル座標系での回転操作
void Transform::RotateLocal(const XMFLOAT3& axis, float angleDeg) {
    XMVECTOR axisVec = XMLoadFloat3(&axis);

    // 回転操作のクオータニオンを計算
    XMVECTOR rotationQuat =
        XMQuaternionRotationAxis(axisVec, XMConvertToRadians(angleDeg));
    // ローカル座標系での回転なので回転をかけてから現在の姿勢を適用する
    XMVECTOR orientation =
        XMQuaternionMultiply(rotationQuat, XMLoadFloat4(&m_orientation));
    XMStoreFloat4(&m_orientation, XMQuaternionNormalize(orientation));
}

// 指定した座標の方向を向く
void Transform::LookAt(const XMFLOAT3& target, const XMFLOAT3& upHint) {
    XMVECTOR eyePos    = XMLoadFloat3(&m_position);
    XMVECTOR targetPos = XMLoadFloat3(&target);

    // 注視方向のベクトルを計算
    XMVECTOR directionVec = targetPos - eyePos;

    // 注視方向ベクトルが小さすぎる場合
    assert(XMVectorGetX(XMVector3LengthSq(directionVec)) > 1e-6f &&
           "target position is too close to the eye position.");

    // LookToを呼び出す
    XMFLOAT3 direction;
    XMStoreFloat3(&direction, directionVec);
    LookTo(direction, upHint);
}

void Transform::LookTo(const XMFLOAT3& direction, const XMFLOAT3& upHint) {
    XMVECTOR directionVec = XMLoadFloat3(&direction);
    XMVECTOR upVec        = XMVector3Normalize(XMLoadFloat3(&upHint));
    assert(XMVectorGetX(XMVector3LengthSq(directionVec)) > 1e-6f &&
           "direction vector is too small.");

    // 基底ベクトルの作成
    XMVECTOR z = XMVector3Normalize(directionVec);  // 前
    XMVECTOR x = XMVector3Cross(upVec, z);          // 右

    // 退化チェック
    if (XMVectorGetX(XMVector3LengthSq(x)) < 1e-6f) {
        // directionとupHintが平行な場合，外積が零ベクトルになり，正規直交基底が求まらない
        // また，平行に近い場合も数値誤差によりrollが不安定になる
        // そのため，代替となるupに切り替える
        upVec = XMLoadFloat3(&engine::kForward);
        x     = XMVector3Cross(upVec, z);
        if (XMVectorGetX(XMVector3LengthSq(x)) < 1e-6f) {
            upVec = XMLoadFloat3(&engine::kRight);
            x     = XMVector3Cross(upVec, z);
        }
    }
    x = XMVector3Normalize(x);

    XMVECTOR y = XMVector3Cross(z, x);  // 上

    // 回転行列の作成
    XMMATRIX rot(x, y, z, XMVectorSet(0, 0, 0, 1));

    // クォータニオンの作成
    XMVECTOR q = XMQuaternionRotationMatrix(rot);

    // 回転の適用
    XMStoreFloat4(&m_orientation, XMQuaternionNormalize(q));
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