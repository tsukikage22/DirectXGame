/// @file BRDF.hlsli
/// @brief BRDFの計算に関する関数群

#pragma once

#ifndef BRDF_HLSLI
#define BRDF_HLSLI

#include "Common.hlsli"

//==============================================================
// Constants
//==============================================================
// 法線と視線の内積の下限値（0除算を防ぐため）
// IBLBake.hlsliと同じ値にする
static const float MIN_NV = 1e-3f;    

//--------------------------------------------------------------
// 5乗の計算
//--------------------------------------------------------------
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

//--------------------------------------------------------------
// Schlickによるフレネル項の近似式
//--------------------------------------------------------------
float3 SchlickFresnel(float3 f0, float cosTheta) {
    return f0 + (1.0f - f0) * pow5((1.0f - cosTheta));
}

//--------------------------------------------------------------
// GGXによる法線分布関数 (D項)
// D(h) = (a^2) / (π * ((N·H)^2 * (a^2 -1) +1)^2 )
//--------------------------------------------------------------
float D_GGX(float NH, float alpha) {
    alpha = max(alpha, 1e-3f); // alphaが0になるのを防ぐ
    float a2 = alpha * alpha;
    float f = (NH * NH) * (a2 - 1.0f) + 1.0f;

    return (a2) / (F_PI * f * f);
}

//--------------------------------------------------------------
// Height-Correlated Smith による減衰幾何項（G項）
//--------------------------------------------------------------
float G2_SmithCorrelated(float NL, float NV, float alpha) {
    float a2 = alpha * alpha;

    NV = max(NV, MIN_NV); // 0除算を防ぐためNVに下限を設定する

    // 可視性関数 V = G / (4 * NL * NV) の形で直接計算する方が効率的
    float GGXV = NL * sqrt(NV * NV * (1.0f - a2) + a2);
    float GGXL = NV * sqrt(NL * NL * (1.0f - a2) + a2);

    return 0.5f / (GGXV + GGXL);
}

//--------------------------------------------------------------
// BRDFの計算（GGX）
//--------------------------------------------------------------
/// @brief BRDFの計算を行う（正規化Lambert + GGX）
/// @note 余弦項は含まない．反射方程式 Lo = f(L,V)*E*(NL)のf(L,V)だけを返す
float3 EvaluateBRDF(float3 N, float3 V, float3 L, float3 baseColor,
    float metallic, float roughness, float3 F0, float3 energyCompensation) {
    // ハーフベクトルの計算
    float3 H = normalize(L + V);

    // 内積
    float NV = saturate(dot(N, V));
    float NL = saturate(dot(N, L));
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    // 拡散反射の計算（正規化Lambertモデル）
    float3 Kd = baseColor.rgb * (1.0f - metallic); // 拡散反射率
    float3 diffuse = Kd * (1.0f / F_PI);

    // 鏡面反射の計算
    float a = roughness * roughness;
    float D = D_GGX(NH, a);
    float G = G2_SmithCorrelated(NL, NV, a);
    float3 Fr = SchlickFresnel(F0, VH);
    float3 specular = D * G * Fr * energyCompensation; // energyCompensationを掛けることで多重散乱の補正を行う

    // 物体の色を反映した最終カラーの計算
    float3 BRDF = diffuse + specular;

    return BRDF;
}

#endif // BRDF_HLSLI