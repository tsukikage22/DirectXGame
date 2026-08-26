/// @file IBLBake.hlsli
/// @brief IBL用のシェーダー共通定義

#pragma once

#ifndef IBLBAKE_HLSLI
#define IBLBAKE_HLSLI

//==============================================================
// Constants
//==============================================================
static const float F_PI = 3.14159265359f; // 円周率

//==============================================================
// Helper Functions
//==============================================================
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
#endif // IBLBAKE_HLSLI
