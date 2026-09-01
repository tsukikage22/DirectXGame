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

/// @brief IBLによる拡散反射と鏡面反射の計算
/// 参考: Fdez-Agüera, "A Multiple-Scattering Microfacet Model for Real-Time Image-based Lighting", 2019
float3 EvaluateIBL(float3 N, float3 V, float3 baseColor, float metallic, 
    float roughness, float ao) {
    // F0の計算
    float3 F0 = lerp(0.04f.xxx, baseColor, metallic);
    float NV = saturate(dot(N, V));
    float3 kS = F_SchlickRoughness(NV, F0, roughness);
    float3 albedo = baseColor.rgb * (1.0f - metallic);

    // BRDF LUTのサンプリング
    float2 f_ab = g_brdfLUT.Sample(g_IBLSampler, float2(NV, roughness)).rg;

    float3 R = reflect(-V, N);      // 反射ベクトルの計算
    float lod = roughness * float(g_scene.prefilteredMipCount - 1); // roughnessに応じてmip levelを選択

    // environment mapのprefiltered mapをサンプリング
    float3 radiance = g_prefilteredMap.SampleLevel(g_IBLSampler, R, lod).rgb; 
    // environment mapのirradiance mapをサンプリング
    float3 irradiance = g_irradianceMap.Sample(g_IBLSampler, N).rgb; 

    // white furnace test
    if(g_scene.debugView == DEBUG_VIEW_WHITE) {
        radiance = 1.0f.xxx;
        irradiance = 1.0f.xxx;
    }

    // 係数の適用
    radiance *= g_scene.envIntensity;  
    irradiance *= g_scene.envIntensity;

    float3 FssEss = f_ab.x * kS + f_ab.y; 

    // 多重散乱
    float Ess = f_ab.x + f_ab.y;    // F0 = 1 の方向アルベド
    float Ems = 1.0f - Ess;         // 失われたエネルギー
    float3 Favg = F0 + (1.0f - F0) * 0.047619f; // 1/21
    float3 FmsEms = Ems * FssEss * Favg / (1.0f - Ems * Favg);

    // 誘電体の拡散項
    float3 kD = albedo * (1.0f - FssEss - FmsEms);

    // 拡散反射と鏡面反射の計算
    float3 specularIBL = radiance * FssEss;              // 単散乱の鏡面
    float3 diffuseIBL  = (FmsEms + kD) * irradiance;     // 多重散乱補填 + 拡散
    return ao * diffuseIBL + specularIBL;
}

#endif // IBL_HLSLI
