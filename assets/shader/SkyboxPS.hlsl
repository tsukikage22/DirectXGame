/// @file SkyboxPS.hlsl
/// @brief スカイボックス描画パス用ピクセルシェーダー

//===============================================================
// Includes
//===============================================================
#include "Common.hlsli"
#include "Display.hlsli"
#include "Tonemap.hlsli"
#include "Skybox.hlsli"

//===============================================================
// Resource Bindings
//===============================================================
TextureCube g_skybox : register(t0);
SamplerState g_sampler : register(s0);

//===============================================================
// ピクセルシェーダーのメイン関数
//===============================================================
float4 main(SkyboxVSOutput input) : SV_TARGET {
    // ワールド座標系の方向ベクトルを正規化
    float3 dir = normalize(input.worldDir);

    // スカイボックスのテクスチャをサンプリング
    float4 color = g_skybox.Sample(g_sampler, dir);

    // 輝度スケール係数
    color *= g_scene.envIntensity; 

    // 露出
    color.rgb *= g_scene.exposure;

    // トーンマッピング
    color.rgb = GT_Tonemap(color.rgb);

    // scRGBに変換
    color.rgb = ToScRGB(color.rgb);

    return float4(color.rgb, 1.0f);

}
