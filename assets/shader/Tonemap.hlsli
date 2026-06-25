/// @file Tonemap.hlsli
/// @brief トーンマッピングに関する関数群

#pragma once

#include "Common.hlsli"

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

    float k = maxLuminance / paperWhiteNits;

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