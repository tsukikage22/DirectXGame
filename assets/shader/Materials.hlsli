/// @file Materials.hlsli
/// @brief マテリアルに関するリソースバインディング

#pragma once

#ifndef MATERIALS_HLSLI
#define MATERIALS_HLSLI

//==============================================================
// Structures
//==============================================================
struct MaterialConstants {
    float4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float3 emissiveFactor;
    float occlusionFactor;
};

//==============================================================
// Resource Bindings
//==============================================================
// [b2] マテリアル定数
ConstantBuffer<MaterialConstants> g_material: register(b2);

// [t0] ベースカラーテクスチャ
Texture2D<float4> baseColorTexture : register(t0);

// [t1] metallic-roughness
Texture2D<float4> metallicRoughnessTexture : register(t1);

// [t2] normal map
Texture2D<float4> normalTexture : register(t2);

// [t3] emissive map
Texture2D<float4> emissiveTexture : register(t3);

// [t4] occlusion map
Texture2D<float4> occlusionTexture : register(t4);

// [s0] サンプラー
SamplerState smp : register(s0);

#endif // MATERIALS_HLSLI