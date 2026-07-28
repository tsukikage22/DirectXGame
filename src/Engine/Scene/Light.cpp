#include "Engine/Scene/Light.h"

#include <algorithm>
#include <cassert>

Light::Light(LightType type)
    : m_type(type),
      m_intensity(100.0f),
      m_range(100.0f),
      m_color({ 1.0f, 1.0f, 1.0f }),
      m_innerAngleDeg(15.0f),
      m_outerAngleDeg(30.0f) {}

Light::~Light() {}

void Light::SetIntensity(float intensity) {
    assert((m_type != LightType::Directional) &&
           "Use SetIlluminance for directional lights.");
    m_intensity = intensity;
}

void Light::SetRange(float range) { m_range = std::max(range, 1e-3f); }

void Light::SetIlluminance(float illuminance) {
    assert((m_type == LightType::Directional) &&
           "Use SetIntensity for non-directional lights.");
    m_intensity = illuminance;
}

//================================
// アクセサ
//================================
void Light::SetColor(const DirectX::XMFLOAT3& color) { m_color = color; }

void Light::SetPosition(const DirectX::XMFLOAT3& position) {
    m_transform.SetPosition(position);
}

void Light::SetRotation(float pitch, float yaw, float roll) {
    m_transform.SetRotation(pitch, yaw, roll);
}

void Light::SetSpotAngles(float innerAngleDeg, float outerAngleDeg) {
    innerAngleDeg   = std::clamp(innerAngleDeg, 0.0f, 90.0f);
    m_outerAngleDeg = std::min(std::max(innerAngleDeg, outerAngleDeg), 90.0f);
    m_innerAngleDeg = std::min(innerAngleDeg, m_outerAngleDeg);
}

Transform& Light::GetTransform() { return m_transform; }

float Light::GetIntensity() const { return m_intensity; }

DirectX::XMFLOAT3 Light::GetColor() const { return m_color; }

float Light::GetRange() const { return m_range; }

LightType Light::GetType() const { return m_type; }

float Light::GetInnerAngle() const { return m_innerAngleDeg; }

float Light::GetOuterAngle() const { return m_outerAngleDeg; }