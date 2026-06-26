/// @file BRDF.hlsli
/// @brief BRDFの計算に関する関数群

#pragma once

#ifndef BRDF_HLSLI
#define BRDF_HLSLI

#include "Common.hlsli"

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
    float a2 = alpha * alpha;
    float f = (NH * NH) * (a2 - 1.0f) + 1.0f;

    return (a2) / (F_PI * f * f);
}

//--------------------------------------------------------------
// Height-Correlated Smith による減衰幾何項（G項）
//--------------------------------------------------------------
float G2_SmithCorrelated(float NL, float NV, float alpha) {
    float a2 = alpha * alpha;

    // 可視性関数 V = G / (4 * NL * NV) の形で直接計算する方が効率的
    float GGXV = NL * sqrt(NV * NV * (1.0f - a2) + a2);
    float GGXL = NV * sqrt(NL * NL * (1.0f - a2) + a2);

    return 0.5f / (GGXV + GGXL + 1e-4f);
}

#endif // BRDF_HLSLI