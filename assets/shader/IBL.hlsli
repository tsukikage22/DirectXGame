/// @file IBL.hlsli
/// @brief IBLに関する定数や構造体の定義

#pragma once

#ifndef IBL_HLSLI
#define IBL_HLSLI

//==============================================================
// Includes
//==============================================================
#include "Common.hlsli"


//==============================================================
// Resource Bindings
//=============================================================
TextureCube<float4> g_irradianceMap : register(t0, space3); // 環境マップのirradiance map
SamplerState g_irradianceSampler : register(s2); // 環境マップのirradiance map用サンプラー


//==============================================================
// Functions
//==============================================================
/// @brief IBLによる拡散反射の計算
/// @param N 法線ベクトル（ワールド座標系）
/// @param baseColor ベースカラー
/// @param metallic メタリック
/// @return IBLによる拡散反射の結果
float3 EvaluateDiffuseIBL(float3 N, float3 baseColor, float metallic) {
    // 環境マップのirradiance mapをサンプリング
    // E/piをirradiance mapに入れたので，そのまま渡せば正規化Lambertのpiで割った値になる
    float3 irradiance = g_irradianceMap.Sample(g_irradianceSampler, N).rgb 
            * g_scene.envIntensity; // 輝度スケール係数を掛ける

    float3 Kd = baseColor.rgb * (1.0f - metallic); // 拡散反射率
    float3 diffuseIBL = irradiance * Kd; // IBLによる拡散反射の計算

    return diffuseIBL;
}


#endif // IBL_HLSLI
