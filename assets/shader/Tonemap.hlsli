/// @file Tonemap.hlsli
/// @brief トーンマッピングに関する関数群

#pragma once

#ifndef TONEMAP_HLSLI
#define TONEMAP_HLSLI

#include "Common.hlsli"
#include "Display.hlsli"

// scRGBは 1.0 = 80nit と定義されている
static const float SC_RGB_WHITE_NITS = 80.0f;

//==============================================================
// Functions
//==============================================================
//--------------------------------------------------------------
// GTトーンマップ
//--------------------------------------------------------------
float3 GT_Tonemap(float3 color) {
    // Max-RGBによる色相シフト防止
    // 色の最大値を代表値として取得し，
    // それにトーンマッピングを適用して他の色はそれとの比率で計算する
    float maxCol = max(max(color.r, color.g), color.b);
    if (maxCol <= 1e-6f)
    {
        return color;
    }

    float k = g_display.maxLuminance / g_display.paperWhiteNits;

    // パラメータ定義
    float P = k;     // 最大輝度
    float a = 1.0f;  // コントラスト
    float m = 0.22f; // 線形区間の開始点
    float l = 0.4f;  // 線形区間の長さ
    float c = 1.33f; // Toeの曲率
    float b = 0.0f;  // 黒浮き補正

    // 係数計算
    float l0 = ((P - m) * l) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    // 区分関数
    float x = maxCol;
    float w0 = 1.0 - smoothstep(0.0f, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0f - w0 - w2;

    // Toe（暗部）
    float T = m * pow(x / m, c) + b;
    // Linear（中間部）
    float L = m + a * (x - m);
    // Shoulder（明部）
    float S = P - (P - S1) * exp(CP * (x - S0));

    // カーブ合成
    float toneMappedMaxCol = T * w0 + L * w1 + S * w2;

    // 色の再構成
    return toneMappedMaxCol * color / maxCol;
}

//--------------------------------------------------------------
// トーンマップ結果（1.0 = paper white）をscRGBに変換する
//--------------------------------------------------------------
float3 ToScRGB(float3 color) {
    // トーンマップの結果は，上限が maxLuminance/paperWhiteNits，1.0がpaperWhiteに相当するが，
    // scRGBでは 1.0 = 80nit と定義されているため，paperWhiteNitsを使ってscRGBに変換する必要がある
    // paperWhiteNits / SC_RGB_WHITE_NITS(80)  を掛けることで scRGBに変換できる
    return color * g_display.paperWhiteNits / SC_RGB_WHITE_NITS;
}

#endif // TONEMAP_HLSLI