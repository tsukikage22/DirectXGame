/// @file ShaderConstants.h
/// @brief シェーダーに渡す定数バッファの構造体
#pragma once

#include <directxmath.h>

namespace shader {

//================================
// フレーム毎に更新する定数
//================================

/// @brief シーン全体に関わる定数（フレーム毎更新）
struct SceneConstants {
    DirectX::XMFLOAT4X4 view;          // ビュー行列
    DirectX::XMFLOAT4X4 projection;    // 射影行列
    DirectX::XMFLOAT3 cameraPosition;  // カメラ位置
    float time;                        // ゲーム時間
    float exposure;                    // 露出調整値
    float _padding[3];                 // 16バイトアラインメント用
};
static_assert(sizeof(SceneConstants) % 16 == 0, "Must be 16-byte aligned");

struct DirectionalLight {
    DirectX::XMFLOAT3 lightDirection;  // 方向
    float lightIntensity;              // 強度
    DirectX::XMFLOAT4 lightColor;      // 色
};
static_assert(sizeof(DirectionalLight) % 16 == 0, "Must be 16-byte aligned");

/// @brief ライティング計算用の定数（フレーム毎更新）
struct LightingConstants {
    uint32_t lightType;               // 0: 平行光源, 1: 点光源, 2: スポット光源
    DirectX::XMFLOAT3 lightPosition;  // 位置（点光源/スポット光源用）
    DirectX::XMFLOAT3 lightDirection;  // 方向（平行光源/スポット光源用）
    float lightIntensity;              // 強度
    DirectX::XMFLOAT3 lightColor;      // 色
    float lightAngleScale;   // スポットライトの角度減衰係数（スポット光源用）
    float lightAngleOffset;  // スポットライトの角度オフセット（スポット光源用）
    float lightInvSqrRadius;  // 距離の二乗の逆数（点光源/スポット光源用）
    float _padding[2];        // 16バイトアラインメント用
};
static_assert(sizeof(LightingConstants) % 16 == 0, "Must be 16-byte aligned");

//================================
// オブジェクト毎に更新する定数
//================================

/// @brief ワールド行列（オブジェクトごと更新）
struct TransformConstants {
    DirectX::XMFLOAT4X4 world;         // ワールド行列
    DirectX::XMFLOAT4X4 worldInverse;  // ワールド逆行列
};
static_assert(sizeof(TransformConstants) % 16 == 0, "Must be 16-byte aligned");

//================================
// マテリアル毎に更新する定数
//================================

/// @brief PBRパラメータ
struct MaterialConstants {
    DirectX::XMFLOAT4 baseColor;
    float metallic;
    float roughness;
    float _padding0;  // 16バイトアラインメント用
    float _padding1;  // 16バイトアラインメント用
    DirectX::XMFLOAT3 emissive;
    float occlusion;
};
static_assert(sizeof(MaterialConstants) % 16 == 0, "Must be 16-byte aligned");

//================================
// ポストプロセス・表示用定数
//================================

/// @brief ディスプレイ情報，ウィンドウ移動時に更新
struct DisplayConstants {
    float maxLuminance;           // 最大輝度
    float minLuminance;           // 最小輝度
    float paperWhiteNits;         // SDRの白の明るさ
    float maxFullFrameLuminance;  // 全白時の最大輝度
};
}  // namespace shader