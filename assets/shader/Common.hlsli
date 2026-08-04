/// @file Common.hlsli
/// @brief 共通で使用する定数や構造体の定義

#pragma once

#ifndef COMMON_HLSLI
#define COMMON_HLSLI

//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f; // 円周率
static const float MIN_DIST = 0.01f;      // 光源との最小距離（距離減衰計算用）

//==============================================================
// VS Output structure
//==============================================================
struct VSOutput
{
    float4 position : SV_POSITION;    // 変換後頂点座標
    float3 worldNormal : TEXCOORD0;   // ワールド座標系の法線
    float2 texCoord : TEXCOORD1;      // テクスチャ座標
    float3 worldPos : TEXCOORD2;      // ワールド座標系の頂点位置
    float3 worldTangent : TEXCOORD3;  // 接線ベクトル
    nointerpolation float handedness : TEXCOORD4;     // 接線空間の右手系/左手系の判定
};

//==============================================================
// PS Output structure
//==============================================================
struct PSOutput
{
    float4 color : SV_TARGET; // 出力カラー
};

//==============================================================
// Constants buffer
//==============================================================
// [b0] シーン定数（View, Projection行列）
cbuffer SceneConstants : register(b0) {
    float4x4 view;    // ビュー行列
    float4x4 proj;    // プロジェクション行列
    float3 cameraPos; // カメラ位置（ワールド座標系）
    float time;       // 経過時間（秒）
    float exposure;   // 露出
    uint lightCount;  // ライトの数
};

// [b2] マテリアル定数
cbuffer MaterialConstants : register(b2) {
    float4 baseColorFactor; // ベースカラー
    float metallicFactor;
    float roughnessFactor;
    float3 emissiveFactor;
    float occlusionFactor;
};

// [b3] ライティング定数
cbuffer LightingConstants : register(b3) {
    uint lightType;          // 0: 平行光源, 1: 点光源, 2: スポット光源
    float3 lightPosition;    // 位置（点光源/スポット光源用）
    float3 lightForward;     // 方向（平行光源/スポット光源用）
    float luminousFlux;      // 光束 [lm]（平行光源以外）
    float illuminance;       // 照度 [lx]（平行光源用）
    float3 lightColor;       // 色
    float lightAngleScale;   // スポットライトの角度減衰係数（スポット光源用）
    float lightAngleOffset;  // スポットライトの角度オフセット（スポット光源用）
    float lightInvSqrRadius; // 計算の打ち切りに使う，影響半径の二乗の逆数（点光源/スポット光源用）
};

// [b4] ディスプレイ定数
cbuffer DisplayConstants : register(b4) {
    float maxLuminance;
    float minLuminance;
    float paperWhiteNits;
    float maxFullFrameLuminance;
};

//==============================================================
// Textures and Samplers
//==============================================================
// [t0] ベースカラーテクスチャ
Texture2D<float4> baseColorTexture : register(t0);

// [t1] metallic-roughness
Texture2D<float4> metallicRoughnessTexture : register(t1);

// [t2] normal map
Texture2D<float4> normalTexture : register(t2);

// [t3] emissive map
Texture2D<float4> emissiveTexture : register(t3);

// [t4] occlusion map
Texture2D<float4> occlusionTexture : register(t4);

// [s0] サンプラー
SamplerState smp : register(s0);

// [t0, space1] IESプロファイルテクスチャ
Texture2D<float4> IESMap : register(t0, space1);

// [s1] IESプロファイル用サンプラー
SamplerState IESSmp : register(s1);

//==============================================================
// Light structure
//==============================================================
// ライト種類
static const uint LIGHT_TYPE_DIRECTIONAL = 0; // 平行光源
static const uint LIGHT_TYPE_POINT = 1;       // 点光源
static const uint LIGHT_TYPE_SPOT = 2;        // スポット光源
static const uint LIGHT_TYPE_PHOTOMETRIC = 3; // フォトメトリックライト

// ライト構造体
struct Light {
    float3 position;    // ライトの位置（ワールド座標系）
    uint type;        // ライトの種類
    float3 forward;     // ライトの方向（ワールド座標系）
    float invSqrRadius; // 影響半径の逆二乗（計算の打ち切りに使う）
    float3 color;       // ライトの色
    float intensity;    // 光の強度（平行光源の場合は照度[lx]，それ以外は光束[lm]）
    float angleScale;   // スポットライトの角度減衰係数
    float angleOffset;  // スポットライトの角度オフセット
    uint iesIndex;      // IESプロファイルのインデックス
    float _padding;  // 16バイトアラインメント用
};

// 一時的な，LightingConstantsからLightを作る関数
Light MakeLightFromLegacy()
{
    Light light;
    light.type = lightType;
    light.position = lightPosition;
    light.color = lightColor;
    light.forward = lightForward;
    light.intensity = (light.type == LIGHT_TYPE_DIRECTIONAL) 
        ? illuminance : luminousFlux;
    light.invSqrRadius = lightInvSqrRadius;
    light.angleOffset = lightAngleOffset;
    light.angleScale = lightAngleScale;
    light.iesIndex = 0;
    return light;
}

#endif // COMMON_HLSLI