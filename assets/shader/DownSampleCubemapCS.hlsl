
/// @file DownSampleCubemapCS.hlsl
/// @brief キューブマップを1段階ダウンサンプリングしてミップを生成する

//==============================================================
// Includes
//==============================================================
#include "IBLBake.hlsli"

//==============================================================
// Resource Bindings
//==============================================================
TextureCube<float4>        g_src: register(t0); // 親ミップのSRV
SamplerState             g_linear   : register(s0);
RWTexture2DArray<float4> g_dst     : register(u0); // 書き込み先ミップのUAV

//==============================================================
// Helper Functions
//==============================================================


//==============================================================
// Compute Shader Entry Point
//==============================================================
[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    // dtid.x, .yはキューブマップの各面の座標に対応するため，範囲外は処理しない
    uint dstW, dstH, faces;
    g_dst.GetDimensions(dstW, dstH, faces);
    if (dtid.x >= dstW || dtid.y >= dstH) return;

    // 出力テクセルの中心に対応する方向ベクトルを取得
    uint face = dtid.z;
    float2 uv = (float2(dtid.xy) + 0.5f) / float2(dstW, dstH); // [0,1]範囲
    float3 dir = TexelToDirection(face, uv); 

    // 親ミップのSRVからサンプリングして出力
    // dstはsrcの半分の解像度なので，dstのuvはsrcの2x2の範囲のちょうど中心に対応する
    // したがってバイリニアサンプリング1回で4テクセルの平均値が得られる
    g_dst[dtid] = float4(g_src.SampleLevel(g_linear, dir, 0.0f).rgb, 1.0f);

}