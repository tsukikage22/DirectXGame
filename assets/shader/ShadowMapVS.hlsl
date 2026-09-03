/// @file ShadowMapVS.hlsl
/// @brief ShadowMap描画用の頂点シェーダ

//===========================================
// Includes
//===========================================
#include "Common.hlsli"

//===========================================
// Structures
//===========================================
/// @brief 頂点シェーダの入力構造体
struct VSInput{
    float3 position : POSITION;     // 頂点座標
};

//===========================================
// Entry Point
//===========================================
float4 main(VSInput input) : SV_POSITION {
    // ライトのクリップ空間へ変換する
    float4 worldPos = mul(float4(input.position, 1.0f), g_transform.world);
    float4 clipPos = mul(worldPos, g_scene.lightViewProj);
    return clipPos;
}