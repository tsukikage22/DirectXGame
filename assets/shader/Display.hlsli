/// @file Display.hlsli
/// @brief DisplayConstants構造体の定義

#pragma once

#ifndef DISPLAY_HLSLI
#define DISPLAY_HLSLI

//=============================================================
// Constants
//=============================================================
// scRGBは 1.0 = 80nit と定義されている
static const float SC_RGB_WHITE_NITS = 80.0f;

//=============================================================
// Structure
//=============================================================
struct DisplayConstants {
    float maxLuminance;
    float minLuminance;
    float paperWhiteNits;
    float maxFullFrameLuminance;
};

//==============================================================
// Resource Bindings
//==============================================================
// [b3] ディスプレイ定数
ConstantBuffer<DisplayConstants> g_display: register(b3);

//==============================================================
// Functions
//==============================================================
//--------------------------------------------------------------
// 1.0をpaper whiteとして正規化された色値をscRGBに変換する
//--------------------------------------------------------------
float3 ToScRGB(float3 color) {
    // トーンマップの結果は，上限が maxLuminance/paperWhiteNits，1.0がpaperWhiteに相当するが，
    // scRGBでは 1.0 = 80nit と定義されているため，paperWhiteNitsを使ってscRGBに変換する必要がある
    // paperWhiteNits / SC_RGB_WHITE_NITS(80)  を掛けることで scRGBに変換できる
    return color * g_display.paperWhiteNits / SC_RGB_WHITE_NITS;
}

//---------------------------------------------------------------
// sRGB -> Linear変換
//---------------------------------------------------------------
float3 SRGBToLinear(float3 srgb) {
    // 0.04045以下の値は12.92で割る
    float3 lo = srgb / 12.92f;
    // 0.04045より大きい値は変換式に従って計算する
    float3 hi = pow((srgb + 0.055f) / 1.055f, 2.4f);
    return lerp(lo, hi, step(0.04045f, srgb));
}

#endif // DISPLAY_HLSLI