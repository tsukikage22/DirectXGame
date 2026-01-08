
//==============================================================
// VS Output structure
//==============================================================
struct VSOutput {
    float4 position : SV_POSITION;  // 変換後頂点座標
    float2 texCoord : TEXCOORD;     // テクスチャ座標
};

//==============================================================
// PS Output structure
//==============================================================
struct PSOutput {
    float4 color : SV_TARGET;       // 出力カラー
};

//==============================================================
// Constants buffer
//==============================================================
// [b2] マテリアル定数
cbuffer MaterialConstants : register(b2) {
    float4 baseColor;    // ベースカラー
    float  metallic;
    float  roughness;
    float3 emissive;
    float  occlusion;
};

// [t0] ベースカラーテクスチャ
Texture2D<float4> baseColorTexture : register(t0);

// [t1] metallic-roughness
Texture2D<float4> metallicRoughnessTexture : register(t1);

// [t2] normal map
Texture2D<float4> normalTexture : register(t2);

// [t3] emissive map
Texture2D<float4> emissiveTexture : register(t3);

// [t4] occlusion map
Texture2D<float4> occlusionTexture : register(t4);

// [s0] サンプラー
SamplerState smp : register(s0);

//==============================================================
// Main function
//==============================================================
PSOutput main(VSOutput input) : SV_TARGET {
    PSOutput output;

    output.color = baseColorTexture.Sample(smp, input.texCoord) * baseColor;

    // デバッグ用: テクスチャ座標をカラーとして表示
    // output.color = float4(input.texCoord, 0.0f, 1.0f);

    return output;
}
