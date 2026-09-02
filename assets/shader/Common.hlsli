/// @file Common.hlsli
/// @brief 共通で使用する定数や構造体の定義

#pragma once

#ifndef COMMON_HLSLI
#define COMMON_HLSLI

//==============================================================
// レジスタ割り当て（ルートシグネチャと対応）
//   b0 SceneConstants      … Common.hlsli
//   b1 TransformConstants  … SceneVS.hlsl
//   b2 MaterialConstants   … Materials.hlsli
//   b3 DisplayConstants    … Tonemap.hlsli
//   t0-t4 / s0  PBRテクスチャ … Materials.hlsli
//   t0 space1 / s1  IES     … Lighting.hlsli
//   t0 space2  ライトバッファ … Lighting.hlsli
//   t0 space3  環境マップのirradiance map … IBL.hlsli
//   t0 space4  シャドウマップ … Shadow.hlsli
//   s3 シャドウマップ用の比較サンプラー … Shadow.hlsli
//==============================================================

//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f; // 円周率
static const float MIN_DIST = 0.01f;      // 光源との最小距離（距離減衰計算用）

// デバッグビュー
static const uint DEBUG_VIEW_FINAL_COLOR = 0;
static const uint DEBUG_VIEW_BASE_COLOR = 1;
static const uint DEBUG_VIEW_NORMAL = 2;
static const uint DEBUG_VIEW_ROUGHNESS = 3;
static const uint DEBUG_VIEW_METALLIC = 4;
static const uint DEBUG_VIEW_AO = 5;
static const uint DEBUG_VIEW_WHITE = 6;
static const uint DEBUG_VIEW_DIFFUSE_IBL = 7;
static const uint DEBUG_VIEW_SPECULAR_IBL = 8;

// シャドウマップを生成するライトの無効値
static const uint INVALID_LIGHT_INDEX = 0xffffffff;

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
    float4x4 invViewProj; // NDCからワールド座標に変換するための行列
    float4x4 lightViewProj; // ライトのビュー射影行列（シャドウマップ用）
    float3 cameraPos;     // カメラ位置（ワールド座標系）
    float time;           // 経過時間（秒）
    float exposure;       // 露出
    uint lightCount;      // ライトの数
    uint debugView;       // 表示モード
    float envIntensity;   // 環境マップの輝度スケール係数
    uint prefilteredMipCount; // prefilteredのmip数
    uint shadowLightIndex; // シャドウマップを生成するライトのインデックス
};

/// @brief ワールド変換行列構造体
struct TransformConstants {
    float4x4 world;
    float4x4 worldInv;
};

//==============================================================
// Resource Bindings
//==============================================================
// [b0] シーン定数（View, Projection行列）
ConstantBuffer<SceneConstants> g_scene: register(b0);

// [b1] ワールド変換行列
ConstantBuffer<TransformConstants> g_transform: register(b1);

#endif // COMMON_HLSLI