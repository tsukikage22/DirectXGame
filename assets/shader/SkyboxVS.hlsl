/// @file SkyboxVS.hlsl
/// @brief スカイボックス描画パス用頂点シェーダー

//===============================================================
// Includes
//===============================================================
#include "Common.hlsli"
#include "Skybox.hlsli"

//---------------------------------------------------------------
// 頂点シェーダーのメイン関数
//---------------------------------------------------------------
SkyboxVSOutput main(uint vid : SV_VertexID) {
    // フルスクリーン三角形：uv = (0, 0), (2, 0), (0, 2)
    float2 uv = float2((vid << 1) & 2, vid & 2);

    // UV座標をNDC座標に変換
    float2 ndc = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

    // NDC座標をクリップ空間に変換
    // z=w にすることで，透視除算後の深度値が1.0fになる
    float4 clipPos = float4(ndc, 1.0f, 1.0f);

    // クリップ空間の座標からワールド空間の位置を計算
    float4 worldPos = mul(clipPos, g_scene.invViewProj);

    // 透視除算を行い，ワールド空間の方向ベクトルを計算
    worldPos /= worldPos.w;
    float3 worldDir = worldPos.xyz - g_scene.cameraPos;

    SkyboxVSOutput output;
    output.position = clipPos;
    output.worldDir = worldDir;
    return output;
}