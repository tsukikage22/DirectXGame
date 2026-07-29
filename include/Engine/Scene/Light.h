/// @file Light.h
/// @brief シーンに配置するライト

#pragma once

#include <DirectXMath.h>

#include <cstdint>

#include "Engine/Scene/Transform.h"

// ライトの種類
enum class LightType : uint32_t {
    Directional,  // 平行光源
    Point,        // 点光源
    Spot,         // スポットライト
    Photometric   // フォトメトリックライト
};

class Light {
public:
    Light(LightType type);
    ~Light();

    /// @brief ライトの有効/無効を切り替える
    void ToggleLight() { m_enabled = !m_enabled; }

    //================================
    // アクセサ
    //================================
    /// @brief 平行光源以外の光の強さを光度[cd]で設定する
    /// @param intensity
    void SetIntensity(float intensity);

    /// @brief 平行光源の照度[lx]を設定する
    /// @param illuminance
    void SetIlluminance(float illuminance);

    void SetRange(float range);
    void SetColor(const DirectX::XMFLOAT3& color);

    void SetSpotAngles(float innerAngleDeg, float outerAngleDeg);

    LightType GetType() const;

    float GetIntensity() const;
    float GetRange() const;
    DirectX::XMFLOAT3 GetColor() const;
    Transform& GetTransform();

    float GetInnerAngle() const;
    float GetOuterAngle() const;

private:
    LightType m_type;  // ライトの種類

    // ライトの汎用パラメータ
    float m_intensity;          // 光の強さ（平行光源はlx，ほかはcd）
    float m_range;              // 範囲
    DirectX::XMFLOAT3 m_color;  // 色
    Transform m_transform;      // ライトの位置・方向を保持するTransform
    bool m_enabled;             // ライトの有効/無効

    // スポットライト用パラメータ
    float m_innerAngleDeg;  // スポットライトの内側角度（度）
    float m_outerAngleDeg;  // スポットライトの外側角度（度）
};