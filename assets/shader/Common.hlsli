/// @file Common.hlsli
/// @brief 共通で使用する定数や構造体の定義

#pragma once

#ifndef COMMON_HLSLI
#define COMMON_HLSLI

//==============================================================
// レジスタ割り当て（ルートシグネチャと対応）
//   b0 SceneConstants      … Common.hlsli
//   b1 TransformConstants  … TestVS.hlsl
//   b2 MaterialConstants   … Materials.hlsli
//   b3 DisplayConstants    … Tonemap.hlsli
//   t0-t4 / s0  PBRテクスチャ … Materials.hlsli
//   t0 space1 / s1  IES     … Lighting.hlsli
//   t0 space2  ライトバッファ … Lighting.hlsli
//==============================================================

//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f; // 円周率
static const float MIN_DIST = 0.01f;      // 光源との最小距離（距離減衰計算用）

//==============================================================
// Structures
//==============================================================
/// @brief 頂点シェーダーの出力構造体
struct VSOutput
{
    float4 position : SV_POSITION;    // 変換後頂点座標
    float3 worldNormal : TEXCOORD0;   // ワールド座標系の法線
    float2 texCoord : TEXCOORD1;      // テクスチャ座標
    float3 worldPos : TEXCOORD2;      // ワールド座標系の頂点位置
    float3 worldTangent : TEXCOORD3;  // 接線ベクトル
    nointerpolation float handedness : TEXCOORD4;     // 接線空間の右手系/左手系の判定
};

/// @brief ピクセルシェーダーの出力構造体
struct PSOutput
{
    float4 color : SV_TARGET; // 出力カラー
};

/// @brief シーン定数構造体
struct SceneConstants
{
    float4x4 view;        // ビュー行列
    float4x4 proj;        // プロジェクション行列
    float3 cameraPos;     // カメラ位置（ワールド座標系）
    float time;           // 経過時間（秒）
    float exposure;       // 露出
    uint lightCount;      // ライトの数
};

//==============================================================
// Resource Bindings
//==============================================================
// [b0] シーン定数（View, Projection行列）
ConstantBuffer<SceneConstants> g_scene: register(b0);

#endif // COMMON_HLSLI