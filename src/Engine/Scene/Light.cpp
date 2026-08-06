#include "Engine/Scene/Light.h"

#include <algorithm>
#include <cassert>
#include <cmath>

Light::Light(LightType type)
    : m_type(type),
      m_range(100.0f),
      m_color({ 1.0f, 1.0f, 1.0f }),
      m_enabled(true),
      m_innerAngleDeg(15.0f),
      m_outerAngleDeg(30.0f) {
    // ライトの種類に応じて初期値を設定
    if (type == LightType::Directional) {
        m_intensity = 10000.0f;  // 照度[lx]
    } else {
        m_intensity = 100.0f;  // 光度[cd]
    }
}

Light::~Light() {}

shader::LightConstants Light::ToShaderConstants() const {
    shader::LightConstants lc = {};
    lc.position               = m_transform.GetPosition();
    lc.forward                = m_transform.GetForward();
    lc.type                   = static_cast<uint32_t>(m_type);
    lc.color                  = m_color;
    lc.intensity              = m_intensity;
    lc.invSqrRadius           = 1.0f / (m_range * m_range);
    lc.iesIndex               = 0;  // IESプロファイルは未対応

    // スポットライトの角度減衰係数とオフセットを計算
    // angleScaleは，ライトベクトルと照射方向のなす角がouterで0，innerで1となる線形補間
    // angleOffsetは，角度がouterのときに0となるように調整するための切片
    if (m_type == LightType::Spot) {
        float cosInner = cosf(DirectX::XMConvertToRadians(m_innerAngleDeg));
        float cosOuter = cosf(DirectX::XMConvertToRadians(m_outerAngleDeg));
        lc.angleScale  = 1.0f / std::max(cosInner - cosOuter, 1e-6f);
        lc.angleOffset = -cosOuter * lc.angleScale;
    }
    return lc;
}

void Light::SetIntensity(float intensity) {
    assert((m_type != LightType::Directional) &&
           "Use SetIlluminance for directional lights.");
    m_intensity = intensity;
}

void Light::SetLuminousFlux(float luminousFlux) {
    assert((m_type != LightType::Directional) &&
           "Use SetIlluminance for directional lights.");
    // 光束[lm]から光度[cd]に変換する
    if (m_type == LightType::Point || m_type == LightType::Photometric) {
        m_intensity = luminousFlux / (4.0f * DirectX::XM_PI);
    } else if (m_type == LightType::Spot) {
        // スポットライトの場合はコーン角を使わずPIで割る（簡易的な近似）
        m_intensity = luminousFlux / DirectX::XM_PI;
    }
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
void Light::SetColor(const DirectX::XMFLOAT3& color) {
    m_color = color;
    // 負の値を許容しない
    m_color.x = std::max(m_color.x, 0.0f);
    m_color.y = std::max(m_color.y, 0.0f);
    m_color.z = std::max(m_color.z, 0.0f);
    // 最大成分で正規化する
    float maxComponent = std::max({ m_color.x, m_color.y, m_color.z });
    if (maxComponent <= 1e-6f) {
        assert(false && "Light color must not be black");
        m_color = { 1.0f, 1.0f, 1.0f };
        return;
    }
    m_color = { m_color.x / maxComponent, m_color.y / maxComponent,
        m_color.z / maxComponent };
}

void Light::SetSpotAngles(float innerAngleDeg, float outerAngleDeg) {
    innerAngleDeg   = std::clamp(innerAngleDeg, 0.0f, 90.0f);
    m_outerAngleDeg = std::min(std::max(innerAngleDeg, outerAngleDeg), 90.0f);
    m_innerAngleDeg = innerAngleDeg;
}

Transform& Light::GetTransform() { return m_transform; }

float Light::GetIntensity() const { return m_intensity; }

DirectX::XMFLOAT3 Light::GetColor() const { return m_color; }

float Light::GetRange() const { return m_range; }

LightType Light::GetType() const { return m_type; }

float Light::GetInnerAngle() const { return m_innerAngleDeg; }

float Light::GetOuterAngle() const { return m_outerAngleDeg; }