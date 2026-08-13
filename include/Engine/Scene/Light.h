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

//================================
// Light Desc Structures
//================================
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

//================================
// Light class
//================================
class Light {
public:
    explicit Light(const DirectionalLightDesc& desc);
    explicit Light(const PointLightDesc& desc);
    explicit Light(const SpotLightDesc& desc);
    explicit Light(const PhotometricLightDesc& desc);

    ~Light();

    /// @brief ライトの有効/無効を切り替える
    void ToggleLight() { m_enabled = !m_enabled; }

    /// @brief ライトの情報をシェーダー用の構造体に変換する
    shader::LightConstants ToShaderConstants() const;

    //================================
    // アクセサ
    //================================
    /// @brief 平行光源以外の光の強さを光度[cd]で設定する
    /// @param intensity
    void SetIntensity(float intensity);

    /// @brief
    /// 平行光源以外の光の強さを光束[lm]で設定する（内部で光度に変換する）
    /// @param luminousFlux
    void SetLuminousFlux(float luminousFlux);

    /// @brief 平行光源の照度[lx]を設定する
    /// @param illuminance
    void SetIlluminance(float illuminance);

    void SetRange(float range);

    /// @brief ライトの色を設定する．色度のみを扱うため内部で正規化を行う．
    /// @param color
    void SetColor(const DirectX::XMFLOAT3& color);

    /// @brief スポットライトの内側角度と外側角度を設定する
    /// @param innerAngleDeg 内側角度（度）
    /// @param outerAngleDeg 外側角度（度）
    void SetSpotAngles(float innerAngleDeg, float outerAngleDeg);

    void SetIESIndex(uint32_t index);

    LightType GetType() const;

    float GetIntensity() const;
    float GetRange() const;
    DirectX::XMFLOAT3 GetColor() const;
    Transform& GetTransform();
    bool IsEnabled() const { return m_enabled; }

    float GetInnerAngle() const;
    float GetOuterAngle() const;

private:
    LightType m_type;  // ライトの種類

    // ライトの汎用パラメータ
    float m_intensity         = 0.0f;   // 光の強さ（平行光源はlx，ほかはcd）
    float m_range             = 30.0f;  // 範囲
    DirectX::XMFLOAT3 m_color = { 1.0f, 1.0f, 1.0f };  // 色
    Transform m_transform;  // ライトの位置・方向を保持するTransform
    bool m_enabled = true;  // ライトの有効/無効

    // スポットライト用パラメータ
    float m_innerAngleDeg = 15.0f;  // スポットライトの内側角度（度）
    float m_outerAngleDeg = 30.0f;  // スポットライトの外側角度（度）

    // フォトメトリックライト用パラメータ
    std::optional<uint32_t> m_iesIndex;  // IESプロファイルのインデックス
};