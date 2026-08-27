/// @file PrefilteredEnvMapCS.hlsl
/// @brief 環境キューブマップからprefiltered env mapを構築する

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
TextureCube<float4>      g_EnvMap   : register(t0);
SamplerState             g_Linear   : register(s0);
RWTexture2DArray<float4> g_PrefilteredEnvMap : register(u0);
cbuffer PrefilterParam : register(b0) {
    float g_roughness;
};

//==============================================================
// Entry Point
//==============================================================
[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    // 範囲外のスレッドは処理しない
    uint dstW, dstH, faces;
    g_PrefilteredEnvMap.GetDimensions(dstW, dstH, faces);
    if(dtid.x >= dstW || dtid.y >= dstH) {
        return;
    }

    // 整数のテクセル位置から，[0,1]のUV座標を計算する
    uint face = dtid.z;
    float2 uv = (float2(dtid.xy) + 0.5f) / float2(dstW, dstH);
    float3 N = normalize(TexelToDirection(face, uv));

    // N=V=Rと仮定する（物体表面の正面から見ていることを前提とする）
    float3 V = N;

    // 環境マップの解像度を取得する
    uint srcW, srcH, srcMips;
    g_EnvMap.GetDimensions(0, srcW, srcH, srcMips);

    // 出力解像度に対応するソースのミップ
    // これより細かいミップを読むと出力側でエイリアスが生じるため，出力解像度に対応するミップを基準にする
    float baseMip = log2(float(srcW) / float(dstW));

    // roughness = 0 のときは畳み込み不要（完全な鏡面反射）
    if(g_roughness <= 0.0f) {
        g_PrefilteredEnvMap[dtid] = float4(g_EnvMap.SampleLevel(g_Linear, N, baseMip).rgb, 1.0f);
        return;
    }

    float alpha = g_roughness * g_roughness;
    // ソースの1テクセルあたりの立体角
    float saTexel = 4.0f * F_PI / (6.0f * float(srcW) * float(srcH));  

    float3 sum = float3(0.0f, 0.0f, 0.0f);
    float weight = 0.0f;

    // 接空間の接線と従法線を計算する
    float3 T, B;
    TangentSpace(N, T, B);

    // サンプリング
    [loop] for(uint i = 0; i < SAMPLE_COUNT; i++) {
        // 乱数を生成する
        float2 Xi = Hammersley(i, SAMPLE_COUNT);

        // GGX分布に従った重点サンプリング
        float3 H = SampleGGX(Xi, alpha, N, T, B); // ハーフベクトル

        // ハーフベクトルから入射方向Lを計算する
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        // 法線と入射方向の内積が負なら，裏側を向いているのでスキップする
        float NL = saturate(dot(N, L));
        if(NL <= 0.0f) continue;

        float NH = saturate(dot(N, H));
        float VH = saturate(dot(V, H));

        // Lの確率密度関数（PDF）を計算する
        float pdf = D_GGX(NH, alpha) * NH / (4.0f * VH) + 1e-4f; // PDFが0になるのを防ぐ
        float saSample = 1.0f / (SAMPLE_COUNT * pdf); // サンプルの面積
        float mip = max(baseMip, 0.5f * log2(saSample / saTexel));

        // 環境マップをサンプリングする
        float3 envColor = g_EnvMap.SampleLevel(g_Linear, L, mip).rgb;

        // サンプルの重みを計算する（余弦項）
        float weightSample = NL;

        sum += envColor * weightSample;
        weight += weightSample;
    }
    g_PrefilteredEnvMap[dtid] = float4(sum / max(weight, 1e-4f), 1.0f);
}