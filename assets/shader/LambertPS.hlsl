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
    float3 worldPos : TEXCOORD2;     // ワールド座標系の頂点位置
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
// [b0] シーン定数（View, Projection行列）
cbuffer SceneConstants: register(b0) {
    float4x4 view;      // ビュー行列
    float4x4 proj;      // プロジェクション行列
    float3 cameraPos;   // カメラ位置（ワールド座標系）
    float time;        // 経過時間（秒）
};

// [b2] マテリアル定数
cbuffer MaterialConstants : register(b2) {
    float4 baseColorFactor;    // ベースカラー
    float  metallicFactor;
    float  roughnessFactor;
    float3 emissiveFactor;
    float  occlusionFactor;
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

    //==============================================
    // 光の計算の準備
    //==============================================
    // テクスチャサンプリング
    float4 baseColorTex = baseColorTexture.Sample(smp, input.texCoord);
    float4 metallicRoughnessTex = metallicRoughnessTexture.Sample(smp, input.texCoord);

    // テクスチャと定数からPBRパラメータを計算
    float4 baseColor = baseColorTex * baseColorFactor;
    float metallic = metallicRoughnessTex.b * metallicFactor;
    float roughness = metallicRoughnessTex.r * roughnessFactor;

    // PBRパラメータから近似値を計算
    float3 diffuseColor = baseColor.rgb * (1.0f - metallic);
    float3 specColor = lerp(0.04, baseColor.rgb, metallic);
    float specPower = lerp(2048, 2, roughness*roughness);



    // ワールド法線正規化
    float3 normal = normalize(input.worldNormal);

    // 視線ベクトルの計算
    float3 viewDir = normalize(cameraPos - input.worldPos);

    // ライトベクトルの正規化
    float3 lightDir = normalize(-directionalLight.lightDirection);

    //==============================================
    // ランバート反射モデルによる拡散反射光計算
    //==============================================
    // 反射率の計算
    // 法線と光源ベクトルの内積（コサイン項）
    float diffuseIntensity = saturate(dot(lightDir, normal));

    // 直接光のエネルギー計算
    // カラー×強度×反射率 
    float3 diffuse = 
        directionalLight.lightColor.rgb * directionalLight.lightIntensity 
        * diffuseIntensity * diffuseColor;

    //==============================================
    // 環境光の計算
    //==============================================
    float3 ambient= ambientColor * ambientIntensity * diffuseColor;

    //==============================================
    // 鏡面反射の計算
    //==============================================
    float3 specular = float3(0.0f, 0.0f, 0.0f);

    // 光が当たっている面だけで鏡面反射を計算
    if(diffuseIntensity > 0.0f) {
        // ハーフベクトルの計算
        float3 halfVec = normalize(lightDir + viewDir);

        // 鏡面反射成分の計算
        float specAngle = saturate(dot(halfVec, normal));
        float specularIntensity = pow(specAngle, specPower);

        // 鏡面反射の色計算
        specular = specColor * directionalLight.lightColor.rgb * directionalLight.lightIntensity * specularIntensity;
    }


    // 物体の色を反映した最終カラーの計算
    float3 finalColor = ambient + diffuse + specular;

    output.color = float4(finalColor, baseColor.a);

    // デバッグ用: テクスチャ座標をカラーとして表示
    // output.color = float4(input.texCoord, 0.0f, 1.0f);

    return output;
}