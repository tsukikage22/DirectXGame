/// @file EquirectToCubemapCS.hlsl
/// @brief HDRIからキューブマップを生成するコンピュートシェーダー


//==============================================================
// Resource Bindings
//==============================================================
Texture2D<float4>        g_Equirect : register(t0);
SamplerState             g_Linear   : register(s0);   // AddressU=WRAP, AddressV=CLAMP
RWTexture2DArray<float4> g_Cube     : register(u0);

//==============================================================
// Constants
//==============================================================
static const float F_PI = 3.14159265359f; // 円周率

//==============================================================
// Helper Functions
//==============================================================
/// @brief キューブマップの各面に対応する方向ベクトルを取得する
/// @param face キューブマップの面番号 (0: +X, 1: -X, 2: +Y, 3: -Y, 4: +Z, 5: -Z)
/// @param uv UV座標 ([0,1]範囲)
/// @return 方向ベクトル
float3 FaceDirection(uint face, float2 uv)   // uv は [0,1]
{
    float2 c = uv * 2.0f - 1.0f;             // [-1,1]範囲に変換 
    switch (face)
    {
        case 0: return float3( 1.0f, -c.y, -c.x); // +X
        case 1: return float3(-1.0f, -c.y,  c.x); // -X
        case 2: return float3( c.x,  1.0f,  c.y); // +Y
        case 3: return float3( c.x, -1.0f, -c.y); // -Y
        case 4: return float3( c.x, -c.y,  1.0f); // +Z
        default:return float3(-c.x, -c.y, -1.0f); // -Z
    }
}

/// @brief 方向ベクトルからEquirectのUV座標を取得する
/// @param dir 方向ベクトル
/// @return UV座標 ([0,1]範囲)
float2 DirectionToEquirectUV(float3 dir)
{
    float2 uv;
    uv.x = atan2(dir.z, dir.x) / (2.0f * F_PI) + 0.5f;
    uv.y = acos(clamp(dir.y, -1.0f, 1.0f)) / F_PI;
    return uv;
}

//==============================================================
// Compute Shader Entry Point
//==============================================================
[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    // dtidからキューブマップの面番号とテクセル座標を取得し，
    // これをUV座標に変換することでキューブマップ上の対応する点へのベクトルを取得する
    // そのベクトルをEquirectのUV座標に変換し，サンプリングすることでキューブマップを生成する

    uint w, h, faces;
    g_Cube.GetDimensions(w, h, faces);
    // dtid.x, .yはキューブマップの各面の座標に対応するため，範囲外は処理しない
    if (dtid.x >= w || dtid.y >= h) return;

    // 整数のテクセル位置から，[0,1]のUV座標を計算する
    float2 uv  = (float2(dtid.xy) + 0.5f) / float2(w, h);

    // uv座標と面番号から，方向ベクトルを取得する
    float3 dir = normalize(FaceDirection(dtid.z, uv));

    // 方向ベクトルからEquirectのUV座標を取得する
    float2 src = DirectionToEquirectUV(dir);

    g_Cube[dtid] = g_Equirect.SampleLevel(g_Linear, src, 0.0f);
}