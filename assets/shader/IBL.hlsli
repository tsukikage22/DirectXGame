/// @file IBL.hlsli
/// @brief IBLに関する定数や構造体の定義

#pragma once

#ifndef IBL_HLSLI
#define IBL_HLSLI

//==============================================================
// Includes
//==============================================================
#include "Common.hlsli"


//==============================================================
// Resource Bindings
//=============================================================
TextureCube<float4> g_irradianceMap : register(t0, space3); // 環境マップのirradiance map
TextureCube<float4> g_prefilteredMap : register(t1, space3); // 環境マップのprefiltered map
Texture2D<float2> g_brdfLUT : register(t2, space3); // 環境マップのBRDF LUT
SamplerState g_IBLSampler : register(s2); // 環境マップのirradiance map用サンプラー


//==============================================================
// Functions
//==============================================================
/// @brief roughnessを考慮したSchlickのフレネル近似式
/// @param cosTheta 法線と視線の角度の余弦値
/// @param F0 反射率
/// @param roughness 粗さ
/// @return フレネル項の計算結果
float3 F_SchlickRoughness(float cosTheta, float3 F0, float roughness) {
    float3 Fr = max((1.0f - roughness).xxx, F0);
    return F0 + (Fr - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}

/// @brief IBLによる拡散反射の計算
/// @param N 法線ベクトル（ワールド座標系）
/// @param baseColor ベースカラー
/// @param Kd 拡散反射率
/// @return IBLによる拡散反射の結果
float3 EvaluateDiffuseIBL(float3 N, float3 baseColor, float3 Kd) {
    // 環境マップのirradiance mapをサンプリング
    // E/piをirradiance mapに入れたので，そのまま渡せば正規化Lambertのpiで割った値になる
    float3 irradiance = (g_scene.debugView == DEBUG_VIEW_WHITE) 
        ? 1.0f.xxx  // white furnace test用のデバッグビュー
        : g_irradianceMap.Sample(g_IBLSampler, N).rgb;

    irradiance *= g_scene.envIntensity; // 輝度スケール係数を掛ける

    float3 diffuseIBL = Kd * irradiance *  baseColor; // IBLによる拡散反射の計算

    return diffuseIBL;
}

float3 EvaluateSpecularIBL(float3 N, float3 V, float roughness, float3 F0) {
    // 反射ベクトルの計算
    float3 R = reflect(-V, N);

    // environment mapのprefiltered mapをサンプリング
    float lod = roughness * float(g_scene.prefilteredMipCount - 1); // roughnessに応じてmip levelを選択
    float3 prefilteredColor = (g_scene.debugView == DEBUG_VIEW_WHITE) 
        ? 1.0f.xxx // white furnace test用のデバッグビュー
        : g_prefilteredMap.SampleLevel(g_IBLSampler, R, lod).rgb;

    prefilteredColor *= g_scene.envIntensity; // 輝度スケール係数を掛ける
    
    // BRDF LUTのサンプリング
    float2 brdfSample = g_brdfLUT.Sample(g_IBLSampler, float2(saturate(dot(N, V)), roughness)).rg;

    // IBLによる鏡面反射の計算
    float3 specularIBL = prefilteredColor * (F0 * brdfSample.x + brdfSample.y);

    return specularIBL;
}

/// @brief IBLによる拡散反射と鏡面反射の計算
float3 EvaluateIBL(float3 N, float3 V, float3 baseColor, float metallic, 
    float roughness, float ao) {
    // F0の計算
    float3 F0 = lerp(0.04f.xxx, baseColor, metallic);
    float3 Ks = F_SchlickRoughness(saturate(dot(N, V)), F0, roughness);
    float3 Kd = (1.0f - Ks) * (1.0f - metallic);

    // IBLによる拡散反射の計算
    float3 diffuseIBL = EvaluateDiffuseIBL(N, baseColor.rgb, Kd);

    // IBLによる鏡面反射の計算
    float3 specularIBL = EvaluateSpecularIBL(N, V, roughness, F0);

    // 拡散反射と鏡面反射を合成して返す
    return ao * diffuseIBL + specularIBL;
}

#endif // IBL_HLSLI
