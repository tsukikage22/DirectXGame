/// @file IBL.hlsli
/// @brief IBLに関する定数や構造体の定義

#pragma once

#ifndef IBL_HLSLI
#define IBL_HLSLI

//==============================================================
// Resource Bindings
//=============================================================
TextureCube<float4> g_irradianceMap : register(t0, space3); // 環境マップのirradiance map
SamplerState g_irradianceSampler : register(s2); // 環境マップのirradiance map用サンプラー




#endif // IBL_HLSLI
