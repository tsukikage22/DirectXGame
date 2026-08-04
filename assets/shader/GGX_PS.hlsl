/// @file GGX_PS.hlsl
/// @brief GGXモデルを使用したPBRのピクセルシェーダ

//==============================================================
// includes
//==============================================================
#include "Common.hlsli"
#include "BRDF.hlsli"
#include "Tonemap.hlsli"
#include "Lighting.hlsli"

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

//==============================================================
// Main function
//==============================================================
PSOutput main(VSOutput input) : SV_TARGET
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
    float4 baseColor = baseColorTex * baseColorFactor;
    float metallic = metallicRoughnessTex.b * metallicFactor;
    float roughness = metallicRoughnessTex.g * roughnessFactor;
    float ao = aoTex * occlusionFactor;

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

    //==============================================
    // 反射計算の準備
    //==============================================
    // viewベクトルの計算
    float3 V = normalize(cameraPos - input.worldPos);

    // 光源からライトベクトルと色付きの照度を取得
    float3 L, E;
    Light light = MakeLightFromLegacy();
    GetLightSample(light, input.worldPos, L, E);

    // 内積計算
    float NL = saturate(dot(N, L));

    //==============================================
    // BRDFの計算
    //==============================================
    float3 BRDF = EvaluateBRDF(N, V, L, baseColor.rgb, metallic, roughness);

    //==============================================
    // 最終カラーの計算 
    //==============================================
    float3 finalColor = E * NL * BRDF;
    finalColor = finalColor * exposure;

    // トーンマップの適用
    float3 toneMapped = GT_Tonemap(finalColor);

    output.color = float4(toneMapped, baseColor.a);

    return output;
}
