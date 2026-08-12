/// @file Lighting.hlsli
/// @brief ライティングに関する宣言や関数

#pragma once

#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "Common.hlsli"

//==============================================================
// Constants
//==============================================================
// ライト種類
static const uint LIGHT_TYPE_DIRECTIONAL = 0; // 平行光源
static const uint LIGHT_TYPE_POINT = 1;       // 点光源
static const uint LIGHT_TYPE_SPOT = 2;        // スポット光源
static const uint LIGHT_TYPE_PHOTOMETRIC = 3; // フォトメトリックライト

//==============================================================
// Structures
//==============================================================
// ライト構造体
struct Light {
    float3 position;    // ライトの位置（ワールド座標系）
    uint type;          // ライトの種類
    float3 forward;     // ライトの方向（ワールド座標系）
    float invSqrRadius; // 影響半径の逆二乗（計算の打ち切りに使う）
    float3 color;       // ライトの色
    float intensity;    // 光の強度（平行光源の場合は照度[lx]，それ以外は光度[cd]）
    float angleScale;   // スポットライトの角度減衰係数
    float angleOffset;  // スポットライトの角度オフセット
    uint iesIndex;      // IESプロファイルのインデックス
    float _padding;     // CPU側構造体とサイズを一致させるため
};

//==============================================================
// Resource Bindings
//==============================================================
// [t0, space2] ライトバッファ
StructuredBuffer<Light> g_lightBuffer : register(t0, space2);

// [t0, space1] IESプロファイルテクスチャ
Texture2DArray<float4> g_IESMaps : register(t0, space1);

// [s1] IESプロファイル用サンプラー
SamplerState g_IESSmp : register(s1);

//==============================================================
// Functions
//==============================================================
//--------------------------------------------------------------
// 距離減衰の計算
//--------------------------------------------------------------
float GetDistanceAttenuation(float3 unnormalizedLightVec, float invSqrRadius) {
    float sqrDist = dot(unnormalizedLightVec, unnormalizedLightVec);
    float invSqr = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
    float window = saturate(1.0f - ((sqrDist * invSqrRadius) * (sqrDist * invSqrRadius)));
    return invSqr * window * window;  
}

//---------------------------------------------------------------
// 角度減衰の計算
//---------------------------------------------------------------
float GetAngleAttenuation(
    float3 lightDir,             // ワールド座標から光源へのベクトル
    float3 lightForward,         // 正規化済みの照射方向ベクトル
    float lightAngleScale,       // スポットライトの角度減衰係数
    float lightAngleOffset       // スポットライトの角度オフセット
) {
    // 以下の値はCPU側で計算する
    // lightAngleScale = 1.0f / max(0.001f, cos(innerConeAngle) -
    // cos(outerConeAngle)); lightAngleOffset = -cos(outerConeAngle) *
    // lightAngleScale;

    float cd = dot(lightForward, -lightDir);
    float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);

    attenuation *= attenuation; // 二乗で滑らかにする

    return attenuation;
}

//--------------------------------------------------------------
// IESプロファイルによる角度減衰の計算（フォトメトリックライト用）
//--------------------------------------------------------------
float GetIESProfileAttenuation(
    float3 lightDir,        // ワールド座標から光源へのベクトル
    float3 lightForward,    // 正規化したライトベクトル
    float iesIndex         // Texture2DArrayのインデックス
) {
    // IESプロファイルテクスチャのUV座標を計算
    // U座標は光源の照射方向と面からライトへの角度の正規化
    float thetaCoord = dot(-lightDir, lightForward) * 0.5f + 0.5f; // [0,1]に正規化

    // V座標は，xy平面上の方位角を計算し，0～1に正規化
    float tangentAngle = atan2(lightDir.y, lightDir.x);
    float phiCoord = (tangentAngle / F_PI) * 0.5f + 0.5f; // [0,1]に正規化

    float3 texCoord = float3(thetaCoord, phiCoord, iesIndex);

    // IESプロファイルテクスチャから正規化された光度をサンプリング
    return g_IESMaps.SampleLevel(g_IESSmp, texCoord, 0).r;
}


/// @brief ライトからライトベクトル（入射方向）Lと照度E[lx]を取り出す
/// @note ここで計算したEは厳密には照度ではない
/// intensityは白色光の時の光度（あるいは照度）という意味で，colorは正規化色度
void GetLightSample(Light light, float3 worldPos, out float3 L, out float3 E) {
    if(light.type == LIGHT_TYPE_DIRECTIONAL) {
        L = -light.forward; // lightの前方の反対
        E = light.color * light.intensity; // intensityは照度，距離減衰無し
        return;
    }

    // ライトベクトルの計算
    float3 toLight = light.position - worldPos;
    L = normalize(toLight);

    // 光度[cd]から照度[lx]への変換
    // 照度は光度*カラー*減衰係数
    float att = GetDistanceAttenuation(toLight, light.invSqrRadius);
    if (light.type == LIGHT_TYPE_SPOT) // スポットライトの場合は角度減衰が必要
    {
        att *= GetAngleAttenuation(L, light.forward, light.angleScale, light.angleOffset);
    }
    else if (light.type == LIGHT_TYPE_PHOTOMETRIC)  // フォトメトリックライトの場合も個別の減衰が必要
    {
        att *= GetIESProfileAttenuation(L, light.forward, light.iesIndex);
    }
    E = light.intensity * light.color * att;

    return;
}

#endif // LIGHTING_HLSLI