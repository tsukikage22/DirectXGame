/// @file Display.hlsli
/// @brief DisplayConstants構造体の定義

#pragma once

#ifndef DISPLAY_HLSLI
#define DISPLAY_HLSLI

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

#endif // DISPLAY_HLSLI