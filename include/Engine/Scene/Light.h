/// @file Light.h
/// @brief シーンに配置するライト

#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <optional>

#include "Engine/Scene/Transform.h"
#include "Engine/Shader/ShaderConstants.h"

// ライトの種類
enum class LightType : uint32_t {
    Directional,  // 平行光源
    Point,        // 点光源
    Spot,         // スポットライト
    Photometric   // フォトメトリックライト
};

//===============================================
// Light Desc Structures
//===============================================
struct DirectionalLightDesc {
    DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };  // ライトの方向
    DirectX::XMFLOAT3 color     = { 1.0f, 1.0f, 1.0f };   // ライトの色
    float illuminance           = 100000.0f;              // 照度[lx]
};

struct PointLightDesc {
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };  // ライトの位置
    DirectX::XMFLOAT3 color    = { 1.0f, 1.0f, 1.0f };  // ライトの色
    float luminousFlux         = 10000.0f;              // 光束[lm]
    float range                = 30.0f;                 // 影響範囲
};

struct SpotLightDesc {
    DirectX::XMFLOAT3 position  = { 0.0f, 0.0f, 0.0f };   // ライトの位置
    DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };  // ライトの方向
    DirectX::XMFLOAT3 color     = { 1.0f, 1.0f, 1.0f };   // ライトの色
    float luminousFlux          = 10000.0f;               // 光束[lm]
    float range                 = 30.0f;                  // 影響範囲
    float innerAngleDeg         = 30.0f;                  // 内側角度（度）
    float outerAngleDeg         = 45.0f;                  // 外側角度（度）
};

struct PhotometricLightDesc {
    DirectX::XMFLOAT3 position  = { 0.0f, 0.0f, 0.0f };   // ライトの位置
    DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };  // ライトの方向
    DirectX::XMFLOAT3 color     = { 1.0f, 1.0f, 1.0f };   // ライトの色
    float luminousFlux          = 10000.0f;               // 光束[lm]
    float range                 = 30.0f;                  // 影響範囲
    std::optional<uint32_t> iesIndex =
        std::nullopt;  // IESプロファイルのインデックス
};

//===============================================
// Light class
//===============================================
class Light {
public:
    //===========================================
    // constructor / destructor
    //===========================================
    explicit Light(const DirectionalLightDesc& desc);
    explicit Light(const PointLightDesc& desc);
    explicit Light(const SpotLightDesc& desc);
    explicit Light(const PhotometricLightDesc& desc);

    ~Light();

    //============================================
    // Conversion to shader constants
    //============================================
    /// @brief ライトの情報をシェーダー用の構造体に変換する
    shader::LightConstants ToShaderConstants() const;

    //============================================
    // Common parameters
    //============================================
    /// @brief 平行光源以外の光の強さを光度[cd]で設定する
    /// @param intensity 光度[cd]
    void SetIntensity(float intensity);

    /// @brief
    /// 平行光源以外の光の強さを光束[lm]で設定する（内部で光度に変換する）
    /// @param luminousFlux 光束[lm]
    void SetLuminousFlux(float luminousFlux);

    /// @brief 平行光源の照度[lx]を設定する
    /// @param illuminance 照度[lx]
    void SetIlluminance(float illuminance);

    /// @brief ライトの影響範囲を設定する
    void SetRange(float range);

    /// @brief ライトの色を設定する．色度のみを扱うため内部で正規化を行う．
    void SetColor(const DirectX::XMFLOAT3& color);

    /// @brief 色温度からライトの色を設定する
    /// @param temperature 色温度[K]，4000K～15000Kの範囲で設定することを想定
    void SetColorFromTemperature(float temperature);

    /// @brief ライトの有効/無効を切り替える
    void ToggleLight() { m_enabled = !m_enabled; }

    LightType GetType() const { return m_type; }
    float GetIntensity() const { return m_intensity; }
    float GetRange() const { return m_range; }
    DirectX::XMFLOAT3 GetColor() const { return m_color; }
    Transform& GetTransform() { return m_transform; }
    bool IsEnabled() const { return m_enabled; }

    //============================================
    // Spot light parameter
    //============================================
    /// @brief スポットライトの内側角度と外側角度を設定する
    /// @param innerAngleDeg 内側角度（度）
    /// @param outerAngleDeg 外側角度（度）
    void SetSpotAngles(float innerAngleDeg, float outerAngleDeg);

    float GetInnerAngle() const { return m_innerAngleDeg; }
    float GetOuterAngle() const { return m_outerAngleDeg; }

    //============================================
    // Photometric light parameter
    //============================================
    void SetIESIndex(std::optional<uint32_t> index) { m_iesIndex = index; }

private:
    LightType m_type;  // ライトの種類

    // ライトの共通パラメータ
    float m_intensity         = 0.0f;   // 光の強さ（平行光源はlx，ほかはcd）
    float m_range             = 30.0f;  // 範囲
    DirectX::XMFLOAT3 m_color = { 1.0f, 1.0f, 1.0f };  // 色度
    Transform m_transform;  // ライトの位置・方向を保持するTransform
    bool m_enabled = true;  // ライトの有効/無効

    // スポットライト用パラメータ
    float m_innerAngleDeg = 15.0f;  // スポットライトの内側角度（度）
    float m_outerAngleDeg = 30.0f;  // スポットライトの外側角度（度）

    // フォトメトリックライト用パラメータ
    std::optional<uint32_t> m_iesIndex;  // IESプロファイルのインデックス
};