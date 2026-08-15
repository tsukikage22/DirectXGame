/// @file UI_PS.hlsl
/// @brief UI合成用ピクセルシェーダー

#include "Display.hlsli"

//===============================================================
// Resource Bindings
//===============================================================
Texture2D<float4> g_uiTexture : register(t0);

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

//---------------------------------------------------------------
// ピクセルシェーダーのメイン関数
//---------------------------------------------------------------
float4 main(float4 pos : SV_Position) : SV_Target {
    // LoadでUIをサンプリング
    float4 uiTex = g_uiTexture.Load(int3(pos.xy, 0));
    // 透明ピクセルを破棄
    if(uiTex.a <= 1e-6f) {
        discard;
    }

    // ImGuiはプリマルチプライ済みのアルファ値を返すため，
    // アンプリマルチプライしてガンマ空間の色値に戻し，
    // EOTFで線形化して白レベルをスケールし，再プリマルチプライする

    // アンプリマルチプライ：ガンマ空間の符号値に戻す
    float3 straight = saturate(uiTex.rgb / uiTex.a);

    // 線形化
    float3 lin = SRGBToLinear(straight);

    // 白レベルをスケールし，再プリマルチプライ
    float3 scaled = ToScRGB(lin);
    return float4(scaled * uiTex.a, uiTex.a);
}