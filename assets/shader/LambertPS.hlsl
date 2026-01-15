//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f;

//==============================================================
// VS Output structure
//==============================================================
struct VSOutput {
    float4 position : SV_POSITION;  // 変換後頂点座標
    float3 worldNormal : TEXCOORD0;  // ワールド座標系の法線
    float2 texCoord : TEXCOORD1;     // テクスチャ座標
};

//==============================================================
// PS Output structure
//==============================================================
struct PSOutput {
    float4 color : SV_TARGET;       // 出力カラー
};

//==============================================================
// Light structure
//==============================================================
struct DirectionalLight {
    float3 lightDirection;  // 方向
    float  lightIntensity;  // 強度
    float4 lightColor;      // 色
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

// [b3] ライティング定数
cbuffer LightingConstants : register(b3) {
    // ambient light
    float3 ambientColor;  // 環境光の色
    float ambientIntensity;          // 環境光の強度

    // directional light
    DirectionalLight directionalLight;
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

    // テクスチャサンプリング
    float4 baseColorTex = baseColorTexture.Sample(smp, input.texCoord);
    float4 albedo = baseColorTex * baseColor;

    // ワールド法線正規化
    float3 normal = normalize(input.worldNormal);

    // ランバート反射モデルによる拡散反射光計算
    // 反射率の計算
    float diffuseIntensity = saturate(dot(-directionalLight.lightDirection, normal));

    // 直接光のエネルギー計算
    // カラー×強度 
    float3 directional = 
        directionalLight.lightColor.rgb * directionalLight.lightIntensity * diffuseIntensity;

    // 環境光の計算
    float3 ambient= ambientColor * ambientIntensity;

    // 物体の色を反映した最終カラーの計算
    float3 finalColor = (directional + ambient) * albedo.rgb  ;

    output.color = float4(finalColor, albedo.a);

    // デバッグ用: テクスチャ座標をカラーとして表示
    // output.color = float4(input.texCoord, 0.0f, 1.0f);

    return output;
}