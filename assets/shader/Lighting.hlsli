/// @file Lighting.hlsli
/// @brief ライティングに関する関数群

#pragma once

#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "Common.hlsli"

//--------------------------------------------------------------
// 距離減衰の計算
//--------------------------------------------------------------
float GetDistanceAttenuation(float3 unnormalizedLightVec) {
    float sqrDist = dot(unnormalizedLightVec, unnormalizedLightVec);
    float invSqr = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
    float window = saturate(1.0f - sqrDist * lightInvSqrRadius);
    return invSqr * window * window; // 二乗で滑らかにする
}

//---------------------------------------------------------------
// 角度減衰の計算
//---------------------------------------------------------------
float GetAngleAttenuation(
    float3 lightDir,             // ライト位置からオブジェクト座標へのベクトル
    float3 lightForward,         // 正規化済みの照射方向ベクトル
    float lightAngleScale,       // スポットライトの角度減衰係数
    float lightAngleOffset       // スポットライトの角度オフセット
) {
    // 以下の値はCPU側で計算する
    // lightAngleScale = 1.0f / max(0.001f, cos(innerConeAngle) -
    // cos(outerConeAngle)); lightAngleOffset = -cos(outerConeAngle) *
    // lightAngleScale;

    float cd = dot(lightForward, lightDir);
    float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);

    attenuation *= attenuation; // 二乗で滑らかにする

    return attenuation;
}

//--------------------------------------------------------------
// IESプロファイルによる角度減衰の計算（フォトメトリックライト用）
//--------------------------------------------------------------
float GetIESProfileAttenuation(
    float3 lightDir,        // ワールド座標から光源へのベクトル
    float3 lightForward     // 正規化したライトベクトル
) {
    // IESプロファイルテクスチャのUV座標を計算
    // U座標は光源の照射方向と面からライトへの角度の正規化
    float thetaCoord = dot(-lightDir, lightForward) * 0.5f + 0.5f; // [0,1]に正規化

    // V座標は，xy平面上の方位角を計算し，0～1に正規化
    float tangentAngle = atan2(lightDir.y, lightDir.x);
    float phiCoord = (tangentAngle / F_PI) * 0.5f + 0.5f; // [0,1]に正規化

    float2 texCoord = float2(thetaCoord, phiCoord);

    // IESプロファイルテクスチャから正規化された光度をサンプリング
    return IESMap.SampleLevel(IESSmp, texCoord, 0).r;
}

//--------------------------------------------------------------
// ポイントライトの計算
//--------------------------------------------------------------
float3 EvaluatePointLight(
    float3 N,         // 法線ベクトル
    float3 worldPos,  // 頂点のワールド座標
    float3 lightPos,  // ライト位置
    float3 lightColor // ライトの色
) {
    float3 dif = lightPos - worldPos;        // オブジェクトから光源へのベクトルを計算
    float3 L = normalize(dif);               // ライトベクトルの正規化
    float att = GetDistanceAttenuation(dif); // 距離減衰の計算

    // 全立体角 4π[sr] で割って光束から光度に変換する
    // I[cd] = Φ[lm] / Ω[sr]
    return saturate(dot(N, L)) * lightColor * att / (4.0f * F_PI);
}

//--------------------------------------------------------------
// スポットライトの計算
//--------------------------------------------------------------
float3 EvaluateSpotLight(
    float3 N,              // 法線ベクトル
    float3 worldPos,       // 頂点のワールド座標
    float3 lightPos,       // ライト位置
    float3 lightForward,       // ライトの照射方向
    float3 lightCol,       // ライトの色
    float angleScale, // スポットライトの角度減衰係数
    float angleOffset // スポットライトの角度オフセット
) {
    float3 unnormalizedLightVec = lightPos - worldPos; // オブジェクトから光源へのベクトルを計算
    float3 L = normalize(unnormalizedLightVec);        // ライトベクトルの正規化
    float att = GetDistanceAttenuation(unnormalizedLightVec);
    att *= GetAngleAttenuation(-L, lightForward, angleScale,
                               angleOffset); // 角度減衰の計算

    // コーン角を使わずにπで割る近似で光束を光度に変換する
    return saturate(dot(N, L)) * lightCol * att / F_PI;
}

//--------------------------------------------------------------
// フォトメトリックライトの計算
//--------------------------------------------------------------
float3 EvaluatePhotometricLight(
    float3 N,              // 法線ベクトル
    float3 worldPos,       // 頂点のワールド座標
    float3 lightPos,       // ライト位置
    float3 lightForward,   // ライトの照射方向
    float3 lightCol        // ライトの色
) {
    float3 unnormalizedLightVec = lightPos - worldPos;
    float3 L = normalize(unnormalizedLightVec);
    float att = 1.0f;
    att *= GetIESProfileAttenuation(L, lightForward);
    att *= GetDistanceAttenuation(unnormalizedLightVec);

    return saturate(dot(N, L)) * lightCol * att / (4.0f * F_PI);
}

#endif // LIGHTING_HLSLI