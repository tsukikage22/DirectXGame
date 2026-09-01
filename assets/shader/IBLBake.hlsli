/// @file IBLBake.hlsli
/// @brief IBL用のシェーダー共通定義

#pragma once

#ifndef IBLBAKE_HLSLI
#define IBLBAKE_HLSLI

//==============================================================
// Constants
//==============================================================
static const float F_PI = 3.14159265359f; // 円周率

// 法線と視線の内積の下限値（0除算を防ぐため）
// BRDF.hlsliと同じ値にする
static const float MIN_NV = 1e-3f;

//==============================================================
// Helper Functions
//==============================================================
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

// Duff et al., "Building an Orthonormal Basis, Revisited", JCGT 6(1), 2017, Listing 3.
// 正規直交基底を計算する
void TangentSpace(in float3 N, out float3 T, out float3 B) {
    float s = (N.z >= 0.0f) ? 1.0f : -1.0f;
    float a = -1.0f / (s + N.z);
    float b = N.x * N.y * a;
    T = float3(1.0f + s * N.x * N.x * a, s * b, -s * N.x);
    B = float3(b, s + N.y * N.y * a, -N.y);
}

// Hammersley点列を生成する
float2 Hammersley(uint i, uint N) {
    float ri = reversebits(i) * 2.3283064365386963e-10f; // 0x100000000
    return float2(float(i) / float(N), ri);
}

// GGXによる法線分布関数 (D項)
// D(h) = (a^2) / (π * ((N·H)^2 * (a^2 -1) +1)^2 )
float D_GGX(float NH, float alpha) {
    alpha = max(alpha, 1e-3f); // alphaが0になるのを防ぐ
    float a2 = alpha * alpha;
    float f = (NH * NH) * (a2 - 1.0f) + 1.0f;

    return (a2) / (F_PI * f * f);
}

// Height-Correlated Smith による減衰幾何項（G項）
float G2_SmithCorrelated(float NL, float NV, float alpha) {
    float a2 = alpha * alpha;

    // 0除算を防ぐためNVに下限を設定する
    NV = max(NV, MIN_NV);

    // 可視性関数 V = G / (4 * NL * NV) の形で直接計算する方が効率的
    float GGXV = NL * sqrt(NV * NV * (1.0f - a2) + a2);
    float GGXL = NV * sqrt(NL * NL * (1.0f - a2) + a2);

    return 0.5f / (GGXV + GGXL);
}

/// @brief キューブマップの各面に対応する方向ベクトルを取得する
/// @param face キューブマップの面番号 (0: +X, 1: -X, 2: +Y, 3: -Y, 4: +Z, 5: -Z)
/// @param uv UV座標 ([0,1]範囲)
/// @return 方向ベクトル
float3 TexelToDirection(uint face, float2 uv)   // uv は [0,1]
{
    float2 c = uv * 2.0f - 1.0f;             // [-1,1]範囲に変換 
    switch (face)
    {
        case 0: return float3( 1.0f, -c.y, -c.x); // +X
        case 1: return float3(-1.0f, -c.y,  c.x); // -X
        case 2: return float3( c.x,  1.0f,  c.y); // +Y
        case 3: return float3( c.x, -1.0f, -c.y); // -Y
        case 4: return float3( c.x, -c.y,  1.0f); // +Z
        default:return float3(-c.x, -c.y, -1.0f); // -Z
    }
}

/// @brief Lambertの余弦則に従った重点サンプリング
/// @param Xi 乱数 ([0,1]範囲)
/// @param N 法線ベクトル
/// @param T 接線ベクトル
/// @param B 従法線ベクトル
/// @return サンプリングされた方向ベクトル
float3 SampleLambert(float2 Xi, float3 N, float3 T, float3 B) {
    float phi = 2.0 * F_PI * Xi.x;
    float cosTheta = sqrt(1.0 - Xi.y);
    float sinTheta = sqrt(Xi.y);

    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // 接空間からワールド空間に変換する
    return normalize(T * H.x + B * H.y + N * H.z);
}


/// @brief GGX分布に従った重点サンプリング
/// @param Xi 
/// @param alpha roughnessの二乗
/// @param N 法線ベクトル
/// @return サンプリングされた方向ベクトル
float3 SampleGGX(float2 Xi, float alpha, float3 N, float3 T, float3 B) {
    float alpha2 = alpha * alpha;
    float phi = 2.0 * F_PI * Xi.x;

    // roughnessが0に近い時，alpha2がfloatの精度で0になりalpha2-1が-1になってしまう
    // その場合，cos2thetaが1を超えてsqrt(1-cos2theta)がNaNになってしまうので，
    // cos2thetaを[0,1]に制限する
    float cos2theta = saturate((1.0 - Xi.y) / (1.0 + (alpha2 - 1.0) * Xi.y));
    float cosTheta = sqrt(cos2theta);
    float sinTheta = sqrt(1.0 - cos2theta);

    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    return normalize(T * H.x + B * H.y + N * H.z);
}


#endif // IBLBAKE_HLSLI
