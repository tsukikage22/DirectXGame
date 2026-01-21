//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f;

//==============================================================
// VS Output structure
//==============================================================
struct VSOutput {
    float4 position : SV_POSITION;      // 変換後頂点座標
    float3 worldNormal : TEXCOORD0;     // ワールド座標系の法線
    float2 texCoord : TEXCOORD1;        // テクスチャ座標
    float3 worldPos : TEXCOORD2;        // ワールド座標系の頂点位置
    float3 worldTangent : TEXCOORD3;    // 接線ベクトル
    float3 worldBinormal: TEXCOORD4;    // 従法線ベクトル
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
// Helper Functions
//==============================================================

//--------------------------------------------------------------
// TBN行列の作成
//--------------------------------------------------------------
float3x3 CreateTBN(float3 normal, float3 tangent, float3 binormal){
    // 正規直交化
    float3 N = normalize(normal);
    float3 T = normalize(tangent);
    T = normalize(T - dot(T, N) * N);
    float3 B = normalize(binormal);
    B = normalize(cross(N, T));

    return float3x3(T, B, N);
}

//--------------------------------------------------------------
// 5乗の計算
//--------------------------------------------------------------
float pow5(float x){
    float x2 = x * x;
    return x2 * x2 * x;
}

//--------------------------------------------------------------
//Schlickによるフレネル項の近似式
//--------------------------------------------------------------
float3 SchlickFresnel(float3 f0, float cosTheta){
    return f0+ (1.0f - f0) * pow5((1.0f - cosTheta));
}

//--------------------------------------------------------------
// BeckMann分布関数
// D(h) = (1/(a^2 * cos^4θ)) * exp( - (tan^2θ)/a^2 )
//--------------------------------------------------------------
float D_Beckmann(float alpha, float cosTheta){
    // alphaが0にならないようにする
    alpha = max(alpha, 1e-4f);

    float c2 = cosTheta * cosTheta;
    float c4 = c2 * c2;
    float a2 = alpha * alpha;
    float t2 = (1.0f - c2) / c2;

    return (1.0f / (a2*c4)) * exp((-1.0f / a2) * t2);
}

//--------------------------------------------------------------
// V-cavityによるシャドウイング・マスキング関数
//--------------------------------------------------------------
float G1_Vcavity(float NH, float NV, float VH){
    VH = max(VH, 1e-4f);
    float g = (2.0f * NH * NV) / VH;
    return min(1.0f, g);
}

float G2_Vcavity(float NH, float NL, float NV, float VH){
    // cosine値が0以下の場合は寄与なし
    if(NV <= 0.0f || NL <= 0.0f){
        return 0.0f;
    }

    float gV = G1_Vcavity(NH, NV, VH);
    float gL = G1_Vcavity(NH, NL, VH);
    return gV * gL;
}

//==============================================================
// Main function
//==============================================================
PSOutput main(VSOutput input) : SV_TARGET {
    PSOutput output;

    //==============================================
    // テクスチャサンプリングとPBRパラメータの計算
    //==============================================
    // テクスチャサンプリング
    float4 baseColorTex = baseColorTexture.Sample(smp, input.texCoord);
    float4 metallicRoughnessTex = metallicRoughnessTexture.Sample(smp, input.texCoord);
    float4 normalTex = normalTexture.Sample(smp, input.texCoord);
    float aoTex = occlusionTexture.Sample(smp, input.texCoord).r;

    // テクスチャと定数からPBRパラメータを計算
    float4 baseColor = baseColorTex * baseColorFactor;
    float metallic = metallicRoughnessTex.b * metallicFactor;
    float roughness = metallicRoughnessTex.r * roughnessFactor;
    float ao = aoTex * occlusionFactor;

    //==============================================
    // 法線ベクトルのワールド変換
    //==============================================
    // 法線マップの値を[-1, 1]の範囲に変換
    float3 tangentSpaceNormal = normalTex.xyz * 2.0f - 1.0f;

    // TBN行列の作成
    // 接空間からワールド空間への変換を行う行列
    float3x3 TBN = CreateTBN(input.worldNormal, input.worldTangent, input.worldBinormal);

    // 法線ベクトルをワールド空間へ変換
    float3 N = normalize(mul(tangentSpaceNormal, TBN));

    //==============================================
    // 反射計算の準備
    //==============================================
    // view, light, halfベクトルの計算
    float3 V = normalize(cameraPos - input.worldPos);
    float3 L = normalize(-directionalLight.lightDirection);
    float3 H = normalize(L + V);

    // 内積
    float NV = saturate(dot(N, V));
    float NL = saturate(dot(N, L));
    float NH = saturate(dot(N, H));
    float VH = saturate(dot(V, H));

    //==============================================
    // 拡散反射の計算（正規化Lambertモデル）
    //==============================================
    float3 Kd = baseColor.rgb * (1.0f - metallic);  // 拡散反射率
    float3 diffuse = Kd * (1.0f / F_PI);

    //==============================================
    // 環境光の計算
    //==============================================
    float3 ambient = ambientColor * ambientIntensity * diffuse;

    //==============================================
    // 鏡面反射の計算
    //==============================================
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metallic);
    float D = D_Beckmann(roughness*roughness, NH);
    float G = G2_Vcavity(NH, NL, NV, VH);
    float3 Fr = SchlickFresnel(F0, NL);

    float3 specular = (D * G * Fr) / (4.0f * NL * NV + 1e-4f);


    // 物体の色を反映した最終カラーの計算
    float3 finalColor = directionalLight.lightIntensity * directionalLight.lightColor.rgb 
        * ( diffuse + specular ) * NL + ambient * ao;

    output.color = float4(finalColor, baseColor.a);

    return output;
}