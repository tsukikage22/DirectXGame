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
    float roughness = metallicRoughnessTex.r * roughnessFactor;
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

    // ライトベクトルの計算
    float3 L;
    if (lightType == 0)
    {
        // ディレクショナルライト
        L = -lightForward;
    }
    else
    {
        // ポイントライトまたはスポットライト
        L = normalize(lightPosition - input.worldPos);
    }

    // ハーフベクトルの計算
    float3 H = normalize(L + V);

    // 内積
    float NV = saturate(dot(N, V));
    float NL = saturate(dot(N, L));
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    //==============================================
    // 拡散反射の計算（正規化Lambertモデル）
    //==============================================
    float3 Kd = baseColor.rgb * (1.0f - metallic); // 拡散反射率
    float3 diffuse = Kd * (1.0f / F_PI);

    //==============================================
    // 鏡面反射の計算
    //==============================================
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metallic);
    float a = roughness * roughness;
    float D = D_GGX(NH, a);
    float G = G2_SmithCorrelated(NL, NV, a);
    float3 Fr = SchlickFresnel(F0, VH);

    float3 specular = D * G * Fr;

    // 物体の色を反映した最終カラーの計算
    float3 BRDF = diffuse + specular;

    // ライティング計算
    float3 lit = float3(0.0f, 0.0f, 0.0f);
    if (lightType == 0)
    {
        // ディレクショナルライト
        lit = NL * lightColor * lightIntensity;
    }
    else if (lightType == 1)
    {
        // ポイントライト
        lit = EvaluatePointLight(N, input.worldPos, lightPosition, lightColor * lightIntensity);
    }
    else if (lightType == 2)
    {
        // スポットライト
        lit = EvaluateSpotLight(N, input.worldPos, lightPosition, lightForward,
                                lightColor * lightIntensity, lightAngleScale, lightAngleOffset);
    }
    else if (lightType == 3) {
        // フォトメトリックライト
        lit = EvaluatePhotometricLight(N, input.worldPos, lightPosition, lightForward,
                                       lightColor * lightIntensity);
    }

    float3 finalColor = lit * BRDF;
    finalColor = finalColor * exposure;

    // トーンマップの適用
    float3 toneMapped = GT_Tonemap(finalColor);

    output.color = float4(toneMapped, baseColor.a);

    return output;
}
