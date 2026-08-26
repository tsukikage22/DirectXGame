/// @file IrradianceCS.hlsl
/// @brief 環境マップから照度マップを生成するコンピュートシェーダー

//==============================================================
// Includes
//==============================================================
#include "IBLBake.hlsli"

//==============================================================
// Resource Bindings
//==============================================================
TextureCube<float4>      g_EnvMap   : register(t0);
SamplerState             g_Linear   : register(s0);
RWTexture2DArray<float4> g_Irradiance : register(u0);

//==============================================================
// Constants
//==============================================================
static const uint SAMPLE_COUNT = 4096; // サンプリング数

//==============================================================
// Compute Shader Entry Point
//==============================================================
[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    // 範囲外のスレッドは処理しない
    uint dstW, dstH, faces;
    g_Irradiance.GetDimensions(dstW, dstH, faces);
    if(dtid.x >= dstW || dtid.y >= dstH) {
        return;
    }

    // 整数のテクセル位置から，[0,1]のUV座標を計算する
    uint face = dtid.z;
    float2 uv = (float2(dtid.xy) + 0.5f) / float2(dstW, dstH);
    float3 N = normalize(TexelToDirection(face, uv));

    // 接空間の正規直交基底を計算する
    float3 T, B;
    TangentSpace(N, T, B);

    float3 sum = float3(0.0f, 0.0f, 0.0f);
    [loop] for(uint i = 0; i<SAMPLE_COUNT; i++) {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);

        // 重点サンプリング（接空間）
        float sinTheta = sqrt(Xi.x);
        float cosTheta = sqrt(1.0f - Xi.x);
        float phi = 2.0f * F_PI * Xi.y;
        float3 Ht = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

        // 接空間からワールド空間に変換する
        float3 L = T * Ht.x + B * Ht.y + N * Ht.z;

        sum += g_EnvMap.SampleLevel(g_Linear, L, 0).rgb;
    }

    // irradiance mapに入れる値は E / pi
    g_Irradiance[dtid] = float4(sum / float(SAMPLE_COUNT), 1.0f);
}