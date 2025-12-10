#include "Engine/Transform.h"

Transform::Transform()
    : m_position{ 0.0f, 0.0f, 0.0f },
      m_rotation{ 0.0f, 0.0f, 0.0f },
      m_scale{ 1.0f, 1.0f, 1.0f } {}

Transform::~Transform() {}

const DirectX::XMMATRIX Transform::CalcWorldMatrix() const {
    // スケーリング
    DirectX::XMMATRIX scaleMatrix =
        DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

    // 回転（Yaw, Pitch, Rollの順）
    // TODO: クォータニオンに移行
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(m_rotation.x),
        DirectX::XMConvertToRadians(m_rotation.y),
        DirectX::XMConvertToRadians(m_rotation.z));

    // 平行移動
    DirectX::XMMATRIX translationMatrix =
        DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

    // ワールド行列の合成（S * R * T）
    DirectX::XMMATRIX worldMatrix =
        scaleMatrix * rotationMatrix * translationMatrix;

    return worldMatrix;
}