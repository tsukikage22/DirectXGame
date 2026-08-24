/// @file GGX_PS.hlsl
/// @brief GGXモデルを使用したPBRのピクセルシェーダ

//==============================================================
// includes
//==============================================================
#include "Common.hlsli"
#include "BRDF.hlsli"
#include "Tonemap.hlsli"
#include "Lighting.hlsli"
#include "Materials.hlsli"

//==============================================================
// Constants
//==============================================================
static const uint DEBUG_VIEW_FINAL_COLOR = 0;
static const uint DEBUG_VIEW_BASE_COLOR = 1;
static const uint DEBUG_VIEW_NORMAL = 2;
static const uint DEBUG_VIEW_ROUGHNESS = 3;
static const uint DEBUG_VIEW_METALLIC = 4;
static const uint DEBUG_VIEW_AO = 5;

//==============================================================
// structures
//==============================================================
//--------------------------------------------------------------
// デバッグビュー選択関数に渡す表面のパラメータをまとめた構造体
//--------------------------------------------------------------
struct SurfaceParams {
    float3 baseColor;
    float metallic;
    float roughness;
    float ao;
    float3 N;
    float3 V;
    float2 uv;
};

//==============================================================
// Helper Functions
//==============================================================
//--------------------------------------------------------------
// TBN行列の作成
//--------------------------------------------------------------
float3x3 CreateTBN(float3 normal, float3 tangent, float handedness) {
    // 正規直交化
    float3 N = normalize(normal);
    float3 T = normalize(tangent);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * handedness;

    return float3x3(T, B, N);
}

//--------------------------------------------------------------
// [0,1]のパラメータをその値のsRGB値として解釈し，表示のためにscRGBに変換する
//--------------------------------------------------------------
float3 ToDebugParam(float3 value) {
    return ToScRGB(SRGBToLinear(saturate(value)));
}

//--------------------------------------------------------------
// デバッグビューの選択
//--------------------------------------------------------------
float3 EvaluateDebugView(SurfaceParams surf, float3 finalColor) {
    switch (g_scene.debugView) {
        case DEBUG_VIEW_FINAL_COLOR:
            return finalColor;
        case DEBUG_VIEW_BASE_COLOR:
            return ToScRGB(surf.baseColor);
        case DEBUG_VIEW_NORMAL:
            return ToDebugParam(surf.N * 0.5f + 0.5f); // [-1, 1] -> [0, 1]
        case DEBUG_VIEW_ROUGHNESS:
            return ToDebugParam(float3(surf.roughness, surf.roughness, surf.roughness));
        case DEBUG_VIEW_METALLIC:
            return ToDebugParam(float3(surf.metallic, surf.metallic, surf.metallic));
        case DEBUG_VIEW_AO:
            return ToDebugParam(float3(surf.ao, surf.ao, surf.ao));
        default:
            return finalColor;
    }
}

//==============================================================
// Main function
//==============================================================
PSOutput main(VSOutput input)  
{
    PSOutput output;

    //==============================================
    // テクスチャサンプリングとPBRパラメータの計算
    //==============================================
    // テクスチャサンプリング
    float4 baseColorTex = baseColorTexture.Sample(smp, input.texCoord);
    float4 metallicRoughnessTex = metallicRoughnessTexture.Sample(smp, input.texCoord);
    float4 normalTex = normalTexture.Sample(smp, input.texCoord);
    float aoTex = occlusionTexture.Sample(smp, input.texCoord).r;

    // テクスチャと定数からPBRパラメータを計算
    float4 baseColor = baseColorTex * g_material.baseColorFactor;
    float metallic = metallicRoughnessTex.b * g_material.metallicFactor;
    float roughness = metallicRoughnessTex.g * g_material.roughnessFactor;
    float ao = aoTex * g_material.occlusionFactor;

    //==============================================
    // 法線ベクトルのワールド変換
    //==============================================
    // 法線マップの値を[-1, 1]の範囲に変換
    float3 tangentSpaceNormal = normalTex.xyz * 2.0f - 1.0f;

    // TBN行列の作成
    // 接空間からワールド空間への変換を行う行列
    float3x3 TBN = CreateTBN(input.worldNormal, input.worldTangent, input.handedness);

    // 法線ベクトルをワールド空間へ変換
    float3 N = normalize(mul(tangentSpaceNormal, TBN));

    // viewベクトルの計算
    float3 V = normalize(g_scene.cameraPos - input.worldPos);

    //==============================================
    // ライティング計算
    //==============================================
    float3 litColor = float3(0.0f, 0.0f, 0.0f);
    
    // 光源の数だけループしてfinalColorに加算
    for(uint i=0; i<g_scene.lightCount; i++) {
        Light light = g_lightBuffer[i];

        // 光源からライトベクトルと色付きの照度を取得
        float3 L, E;
        GetLightSample(light, input.worldPos, L, E);

        // 内積計算
        float NL = saturate(dot(N, L));

        // BRDFの計算
        float3 BRDF = EvaluateBRDF(N, V, L, baseColor.rgb, metallic, roughness);
        litColor += E * NL * BRDF;
    }

    litColor *= g_scene.exposure;

    // トーンマップの適用
    float3 toneMapped = GT_Tonemap(litColor);

    // scRGBに変換
    toneMapped = ToScRGB(toneMapped);

    // デバッグビューの選択
    SurfaceParams surf;
    surf.baseColor = baseColor.rgb;
    surf.metallic = metallic;
    surf.roughness = roughness;
    surf.ao = ao;
    surf.N = N;
    surf.V = V;
    float3 finalColor = EvaluateDebugView(surf, toneMapped);

    output.color = float4(finalColor, baseColor.a);

    return output;
}
