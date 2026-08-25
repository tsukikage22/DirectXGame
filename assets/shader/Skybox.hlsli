/// @file Skybox.hlsli
/// @brief スカイボックス描画パス用共通定義

#pragma once

#ifndef SKYBOX_HLSLI
#define SKYBOX_HLSLI

//===============================================================
// structures
//===============================================================
struct SkyboxVSOutput {
    float4 position : SV_POSITION; // 変換後頂点座標
    float3 worldDir : TEXCOORD0;   // ワールド座標系の方向ベクトル
};

#endif // SKYBOX_HLSLI
