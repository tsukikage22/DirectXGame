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
    float3 worldBinormal : TEXCOORD4; // 従法線ベクトル
};

//==============================================================
// PS Output structure
//==============================================================
struct PSOutput
{
    float4 color : SV_TARGET; // 出力カラー
};

//==============================================================
// Light structure
//==============================================================
struct DirectionalLight
{
    float3 lightDirection; // 方向
    float lightIntensity;  // 強度
    float4 lightColor;     // 色
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
    float lightIntensity;    // 強度
    float3 lightColor;       // 色
    float lightAngleScale;   // スポットライトの角度減衰係数（スポット光源用）
    float lightAngleOffset;  // スポットライトの角度オフセット（スポット光源用）
    float lightInvSqrRadius; // 距離の二乗の逆数（点光源/スポット光源用）
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

#endif // COMMON_HLSLI