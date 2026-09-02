/// @file Shadow.hlsli
/// @brief シャドウマップに関する定数や構造体の定義

#pragma once

#ifndef SHADOW_HLSLI
#define SHADOW_HLSLI

//==============================================================
// Resource Bindings
//==============================================================
Texture2D<float> g_shadowMap : register(t0, space4); // シャドウマップ
SamplerComparisonState g_shadowSampler : register(s3); // シャドウマップ用の比較サンプラー


//==============================================================
// functions
//==============================================================
/// @brief シャドウマップの計算
float ComputeShadow(float3 worldPos) {
    // 描画対象の座標をワールドからライト空間に変換し，
    // シャドウマップのUV座標へ変換する
    // そしてシャドウマップの深度値と比較して，影の有無を計算する

    // ワールド座標をライト空間に変換
    float4 lightClipPos = mul(float4(worldPos, 1.0f), g_scene.lightViewProj);

    // ライトのNDC座標に変換
    float3 lightNDCPos = lightClipPos.xyz / lightClipPos.w;

    // ライトのNDC座標を[0, 1]の範囲に変換
    float2 shadowMapUV = lightNDCPos.xy * float2(0.5f, -0.5f) + 0.5f;

    // farより遠い場合は影の計算を行わない
    if (lightNDCPos.z > 1.0f) {
        return 1.0f;
    }

    // シャドウマップの深度値を取得
    float shadowMapDepth = g_shadowMap.SampleCmpLevelZero(g_shadowSampler, 
        shadowMapUV, lightNDCPos.z);

    return shadowMapDepth;
}

#endif // SHADOW_HLSLI