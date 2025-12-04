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
};
static_assert(sizeof(SceneConstants) % 16 == 0, "Must be 16-byte aligned");

/// @brief ライティング計算用の定数（フレーム毎更新）
struct LightingConstants {
    DirectX::XMFLOAT3 lightDirection;  // 方向
    float lightIntensity;              // 強度
    DirectX::XMFLOAT4 lightColor;      // 色
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
    DirectX::XMFLOAT3 emissive;
    float occlusion;
    float _padding0;  // 16バイトアラインメント用
    float _padding1;  // 16バイトアラインメント用
};
static_assert(sizeof(MaterialConstants) % 16 == 0, "Must be 16-byte aligned");
}  // namespace shader