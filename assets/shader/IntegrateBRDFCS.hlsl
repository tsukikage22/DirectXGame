/// @file IntegrateBRDFCS.hlsl
/// @brief BRDF LUTを構築する

//==============================================================
// Includes
//==============================================================
#include "IBLBake.hlsli"

//==============================================================
// Constants
//==============================================================
static const uint SAMPLE_COUNT = 1024; // サンプリング数

//==============================================================
// Resource Bindings
//==============================================================
RWTexture2D<float2> g_BrdfLut: register(u0);

//==============================================================
// Functions
//==============================================================

//==============================================================
// Entry Point
//==============================================================
[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    // 範囲外のスレッドは処理しない
    uint dstW, dstH;
    g_BrdfLut.GetDimensions(dstW, dstH);
    if(dtid.x >= dstW || dtid.y >= dstH) {
        return;
    }

    // 整数のテクセル位置から，[0,1]のUV座標を計算する
    float2 uv = (float2(dtid.xy) + 0.5f) / float2(dstW, dstH);

    // x軸: N·V, y軸: roughness
    float NV = uv.x;
    float roughness = uv.y;
    float alpha = roughness * roughness;

    // 法線をZ軸方向に固定し，VをXZ平面上に置く
    float3 N = float3(0.0f, 0.0f, 1.0f);
    float3 T = float3(1.0f, 0.0f, 0.0f);
    float3 B = float3(0.0f, 1.0f, 0.0f);
    float3 V = float3(sqrt(1.0f - NV * NV), 0.0f, NV);

    float A = 0.0f;
    float Bsum = 0.0f;
    [loop] for(uint i=0; i<SAMPLE_COUNT; ++i) {
        // 乱数を生成する
        float2 Xi = Hammersley(i, SAMPLE_COUNT);

        // GGX分布に従ったハーフベクトルのサンプリング
        float3 H = SampleGGX(Xi, alpha, N, T, B);

        // サンプリング方向の計算
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NL = saturate(dot(N, L));
        float NH = saturate(dot(N, H));
        float VH = saturate(dot(V, H));
        if(NL <= 0.0f || NH <= 0.0f) continue;

        // D項が約分された形，Visは G/(4*NL*NV) 
        float Vis = G2_SmithCorrelated(NL, NV, alpha);
        float weight = 4.0f * Vis * NL * VH / max(NH, 1e-4f);

        float Fc = pow5(1.0f - VH);
        A += (1.0f - Fc) * weight;
        Bsum += Fc * weight;

    }

    g_BrdfLut[dtid.xy] = float2(A, Bsum) / float(SAMPLE_COUNT);


}