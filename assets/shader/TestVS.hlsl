/// @file TestVS.hlsl
/// @brief 頂点シェーダ

#include "Common.hlsli"

//===========================================
// Structures
//===========================================
/// @brief 頂点シェーダの入力構造体
struct VSInput{
    float3 position : POSITION;     // 頂点座標
    float3 normal   : NORMAL;       // 頂点法線
    float4 tangent  : TANGENT;      // 接線ベクトル
    float2 texCoord : TEXCOORD;     // テクスチャ座標
    float4 color    : COLOR;        // 頂点カラー
};

/// @brief ワールド変換行列構造体
struct TransformConstants {
    float4x4 world;
    float4x4 worldInv;
};


//===========================================
// constants buffer
//===========================================
// [b1] ワールド変換行列
ConstantBuffer<TransformConstants> g_transform: register(b1);

VSOutput main(VSInput input) {
    VSOutput output;

    // 1. ローカル座標 -> ワールド座標変換
    float4 worldPos = mul(float4(input.position, 1.0f), g_transform.world);
    output.worldPos = worldPos.xyz;

    // 2. ワールド座標 -> ビュー座標変換
    float4 viewPos = mul(worldPos, g_scene.view);

    // 3. ビュー座標 -> 射影変換
    output.position = mul(viewPos, g_scene.proj);

    // UV座標の受け渡し
    output.texCoord = input.texCoord;

    // 法線のワールド座標系への変換
    float3 worldNormal = mul(input.normal, (float3x3)g_transform.worldInv);
    output.worldNormal = normalize(worldNormal);

    // 接線ベクトルのワールド座標系への変換
    float3 worldTangent = mul(input.tangent.xyz, (float3x3)g_transform.world);
    output.worldTangent = normalize(worldTangent);

    // 接線空間の右手系/左手系の判定
    output.handedness = input.tangent.w;

    return output;
}