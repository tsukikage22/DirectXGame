#include "Engine/Scene/Light.h"

#include <algorithm>
#include <cassert>
#include <cmath>

//========================================
// Constructors and Destructor
//========================================
Light::Light(const DirectionalLightDesc& desc) : m_type(LightType::Directional)
{
    SetColor(desc.color);
    SetIlluminance(desc.illuminance);
    m_transform.LookTo(desc.direction);
}

Light::Light(const PointLightDesc& desc) : m_type(LightType::Point)
{
    SetRange(desc.range);
    SetColor(desc.color);
    SetLuminousFlux(desc.luminousFlux);
    m_transform.SetPosition(desc.position);
}

Light::Light(const SpotLightDesc& desc) : m_type(LightType::Spot)
{
    SetSpotAngles(desc.innerAngleDeg, desc.outerAngleDeg);
    SetRange(desc.range);
    SetColor(desc.color);
    SetLuminousFlux(desc.luminousFlux);
    m_transform.SetPosition(desc.position);
    m_transform.LookTo(desc.direction);
}

Light::Light(const PhotometricLightDesc& desc) : m_type(LightType::Photometric)
{
    assert(desc.iesIndex.has_value() && "IES index must be provided for photometric lights.");

    SetIESIndex(desc.iesIndex.value());
    SetRange(desc.range);
    SetColor(desc.color);
    SetLuminousFlux(desc.luminousFlux);
    m_transform.SetPosition(desc.position);
    m_transform.LookTo(desc.direction);
}

Light::~Light()
{
}

//========================================
// Conversion to shader constants
//========================================
shader::LightConstants Light::ToShaderConstants() const
{
    shader::LightConstants lc = {};
    lc.position               = m_transform.GetPosition();
    lc.forward                = m_transform.GetForward();
    lc.type                   = static_cast<uint32_t>(m_type);
    lc.color                  = m_color;
    lc.intensity              = m_intensity;
    lc.invSqrRadius           = 1.0f / (m_range * m_range);

    // スポットライトの角度減衰係数とオフセットを計算
    // angleScaleは，ライトベクトルと照射方向のなす角がouterで0，innerで1となる線形補間
    // angleOffsetは，角度がouterのときに0となるように調整するための切片
    if (m_type == LightType::Spot)
    {
        float cosInner = cosf(DirectX::XMConvertToRadians(m_innerAngleDeg));
        float cosOuter = cosf(DirectX::XMConvertToRadians(m_outerAngleDeg));
        lc.angleScale  = 1.0f / std::max(cosInner - cosOuter, 1e-6f);
        lc.angleOffset = -cosOuter * lc.angleScale;
    }

    // IESプロファイルのインデックスを設定
    if (m_type == LightType::Photometric)
    {
        assert(m_iesIndex.has_value() && "IES profile index is not set.");
        lc.iesIndex = m_iesIndex.value();
    }

    return lc;
}

//================================
// Common parameters
//================================
void Light::SetIntensity(float intensity)
{
    assert((m_type != LightType::Directional) && "Use SetIlluminance for directional lights.");
    m_intensity = intensity;
}

void Light::SetLuminousFlux(float luminousFlux)
{
    assert((m_type != LightType::Directional) && "Use SetIlluminance for directional lights.");
    // 光束[lm]から光度[cd]に変換する
    if (m_type == LightType::Point || m_type == LightType::Photometric)
    {
        m_intensity = luminousFlux / (4.0f * DirectX::XM_PI);
    }
    else if (m_type == LightType::Spot)
    {
        // スポットライトの場合はコーン角を使わずPIで割る（簡易的な近似）
        m_intensity = luminousFlux / DirectX::XM_PI;
    }
}

void Light::SetIlluminance(float illuminance)
{
    assert((m_type == LightType::Directional) && "Use SetIntensity for non-directional lights.");
    m_intensity = illuminance;
}

void Light::SetRange(float range)
{
    m_range = std::max(range, 1e-3f);
}

void Light::SetColor(const DirectX::XMFLOAT3& color)
{
    m_color = color;
    // 負の値を許容しない（色域外の色は簡易的にクリップする）
    m_color.x = std::max(m_color.x, 0.0f);
    m_color.y = std::max(m_color.y, 0.0f);
    m_color.z = std::max(m_color.z, 0.0f);

    // 相対輝度（Rec.709）で正規化する
    // intensityは視感度で重み付けされた量なので，
    // 色度を変えても明るさが保たれるように正規化する必要がある
    constexpr DirectX::XMFLOAT3 kRec709Luminance = { 0.2126f, 0.7152f, 0.0722f };
    float luminance = m_color.x * kRec709Luminance.x + m_color.y * kRec709Luminance.y + m_color.z * kRec709Luminance.z;
    if (luminance < 1e-6f)
    {
        assert(false && "Light color must not be black");
        m_color = { 1.0f, 1.0f, 1.0f };
        return;
    }
    m_color = { m_color.x / luminance, m_color.y / luminance, m_color.z / luminance };
}

void Light::SetColorFromTemperature(float temperature)
{
    // 色温度の範囲を制限
    assert((temperature >= 4000.0f && temperature <= 15000.0f) &&
           "Color temperature should be in the range of 4000K to 15000K.");

    // 色温度からxy色度への変換
    float x, y;
    if (temperature <= 7000.0f)
    {
        x = -4.6070e9f / (temperature * temperature * temperature) + 2.9678e6f / (temperature * temperature) +
            0.09911e3f / temperature + 0.244063f;
    }
    else
    {
        x = -2.0064e9f / (temperature * temperature * temperature) + 1.9018e6f / (temperature * temperature) +
            0.24748e3f / temperature + 0.237040f;
    }
    y = -3.000f * x * x + 2.870f * x - 0.275f;

    // xy色度からXYZ色度への変換
    float Y = 1.0f; // 明度は1.0に固定
    float X = (Y / y) * x;
    float Z = (Y / y) * (1.0f - x - y);

    // XYZ色度からsRGBへの変換
    float r = 3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b = 0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;

    SetColor({ r, g, b });
}

const char* Light::GetTypeName() const
{
    switch (m_type)
    {
    case LightType::Directional:
        return "Directional";
    case LightType::Point:
        return "Point";
    case LightType::Spot:
        return "Spot";
    case LightType::Photometric:
        return "Photometric";
    default:
        return "Unknown";
    }
}

//================================
// Spot light parameter
//================================
void Light::SetSpotAngles(float innerAngleDeg, float outerAngleDeg)
{
    innerAngleDeg   = std::clamp(innerAngleDeg, 0.0f, 90.0f);
    m_outerAngleDeg = std::min(std::max(innerAngleDeg, outerAngleDeg), 90.0f);
    m_innerAngleDeg = innerAngleDeg;
}
