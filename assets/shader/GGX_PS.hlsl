
//==============================================
// Constant Values
//==============================================
static const float F_PI = 3.14159265359f;   // 円周率
static const float MIN_DIST = 0.01f;        // 光源との最小距離（距離減衰計算用）

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
    float exposure;    // 露出
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
    uint lightType;               // 0: 平行光源, 1: 点光源, 2: スポット光源
    float3 lightPosition;  // 位置（点光源/スポット光源用）
    float3 lightDirection;  // 方向（平行光源/スポット光源用）
    float lightIntensity;              // 強度
    float3 lightColor;      // 色
    float lightAngleScale;   // スポットライトの角度減衰係数（スポット光源用）
    float lightAngleOffset;  // スポットライトの角度オフセット（スポット光源用）
    float lightInvSqrRadius;  // 距離の二乗の逆数（点光源/スポット光源用）
};

// [b4] ディスプレイ定数
cbuffer DisplayConstants : register(b4) {
    float maxLuminance;
    float minLuminance;
    float paperWhiteNits;
    float maxFullFrameLuminance;
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
// Schlickによるフレネル項の近似式
//--------------------------------------------------------------
float3 SchlickFresnel(float3 f0, float cosTheta){
    return f0+ (1.0f - f0) * pow5((1.0f - cosTheta));
}

//--------------------------------------------------------------
// GGXによる法線分布関数 (D項)
// D(h) = (a^2) / (π * ((N·H)^2 * (a^2 -1) +1)^2 )
//--------------------------------------------------------------
float D_GGX(float NH, float alpha){
    float a2 = alpha * alpha;
    float f = (NH * NH) * (a2 - 1.0f) + 1.0f;

    return (a2) / (F_PI * f * f);
}

//--------------------------------------------------------------
// Height-Correlated Smith による減衰幾何項（G項）
//--------------------------------------------------------------
float G2_SmithCorrelated(float NL, float NV, float alpha){
    float a2 = alpha * alpha;
    
    // 可視性関数 V = G / (4 * NL * NV) の形で直接計算する方が効率的
    float GGXV = NL * sqrt(NV * NV * (1.0f - a2) + a2);
    float GGXL = NV * sqrt(NL * NL * (1.0f - a2) + a2);
    
    return 0.5f / (GGXV + GGXL + 1e-4f);
}

//--------------------------------------------------------------
// GTトーンマップ
//--------------------------------------------------------------
float3 GT_Tonemap(float3 color){
    // Max-RGBによる色相シフト防止
    // 色の最大値を代表値として取得し，
    // それにトーンマッピングを適用して他の色はそれとの比率で計算する
    float maxCol = max(max(color.r, color.g), color.b);
    if(maxCol <= 1e-6f){
        return color;
    }

    float k = maxLuminance / paperWhiteNits;

    // パラメータ定義
    float P = k;        // 最大輝度
    float a = 1.0f;     // コントラスト
    float m = 0.22f;    // 線形区間の開始点
    float l = 0.4f;     // 線形区間の長さ
    float c = 1.33f;    // Toeの曲率
    float b = 0.0f;     // 黒浮き補正

    // 係数計算
    float l0 = ((P-m) * l) / a;
    float S0 = m + l0;
    float S1 = m + a *l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    // 区分関数
    float x = maxCol;
    float w0 = 1.0 - smoothstep(0.0f, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0f - w0 - w2;

    // Toe（暗部）
    float T = m * pow(x / m, c) + b;
    // Linear（中間部）
    float L = m + a * (x - m);
    // Shoulder（明部）
    float S = P - (P - S1) * exp(CP * (x - S0));

    // カーブ合成
    float toneMappedMaxCol = T * w0 + L * w1 + S * w2;

    // 色の再構成
    return toneMappedMaxCol * color / maxCol;
}

//--------------------------------------------------------------
// 距離減衰の計算（点光源用）
//--------------------------------------------------------------
float GetDistanceAttenuation(float3 unnormalizedLightVec){
    float sqrDist = dot(unnormalizedLightVec, unnormalizedLightVec);
    float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
    return saturate(1.0f - sqrDist* lightInvSqrRadius);
}

//---------------------------------------------------------------
// 角度減衰の計算（スポット光源用）
//---------------------------------------------------------------
float GetAngleAttenuation(
    float3 unnormalizedLightVec,    // ライト位置からオブジェクト座標へのベクトル
    float3 lightDir,                // 正規化済みの照射方向ベクトル
    float lightAngleScale,          // スポットライトの角度減衰係数
    float lightAngleOffset          // スポットライトの角度オフセット
){
    // 以下の値はCPU側で計算する
    // lightAngleScale = 1.0f / max(0.001f, cos(innerConeAngle) - cos(outerConeAngle));
    // lightAngleOffset = -cos(outerConeAngle) * lightAngleScale;

    float cd = dot(lightDir, unnormalizedLightVec);
    float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);
    
    attenuation *= attenuation; // 二乗で滑らかにする

    return attenuation;
}

//--------------------------------------------------------------
// 点光源によるライティング計算
//--------------------------------------------------------------
float3 EvaluatePointLight
    (
        float3 N,           // 法線ベクトル
        float3 worldPos,    // 頂点のワールド座標
        float3 lightPos,    // 光源位置
        float3 lightColor   // 光の色
    )
{
    float3 dif = lightPos - worldPos;   // オブジェクトから光源へのベクトルを計算
    float3 L = normalize(dif);      // ライトベクトルの正規化
    float att = GetDistanceAttenuation(dif);    // 距離減衰の計算

    return saturate(dot(N, L)) * lightColor * att / (4.0f * F_PI);
}

//--------------------------------------------------------------
// スポット光源によるライティング計算
//--------------------------------------------------------------
float3 EvaluateSpotLight(
    float3 N,               // 法線ベクトル
    float3 worldPos,        // 頂点のワールド座標
    float3 lightPos,        // 光源位置
    float3 lightDir,        // 光源の照射方向
    float3 lightCol,        // 光の色
    float lightAngleScale,  // スポットライトの角度減衰係数
    float lightAngleOffset  // スポットライトの角度オフセット
){
    float3 unnormalizedLightVec = lightPos - worldPos;   // オブジェクトから光源へのベクトルを計算
    float3 L = normalize(unnormalizedLightVec);      // ライトベクトルの正規化
    float sqrDist = dot(unnormalizedLightVec, unnormalizedLightVec);
    float att = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));    // 距離減衰の計算
    att *= GetAngleAttenuation(-unnormalizedLightVec, lightDir, lightAngleScale, lightAngleOffset);  // 角度減衰の計算
    return saturate(dot(N, L)) * lightCol * att / F_PI;
    
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
    float3 L = normalize(lightPosition - input.worldPos);
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
    // 鏡面反射の計算
    //==============================================
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metallic);
    float a = roughness * roughness;
    float D = D_GGX(NH, a);
    float G = G2_SmithCorrelated(NL, NV, a);
    float3 Fr = SchlickFresnel(F0, VH);

    float3 specular = D * G * Fr;  


    // 物体の色を反映した最終カラーの計算
    float3 BRDF = diffuse + specular;

    // ライティング計算
    float3 lit = float3(0.0f, 0.0f, 0.0f);
    if( lightType == 1 ) {
        // ポイントライト
        lit = EvaluatePointLight(N, input.worldPos, lightPosition, 
            lightColor * lightIntensity);
    } else if( lightType == 2 ) {
        // スポットライト
        lit = EvaluateSpotLight(N, input.worldPos, lightPosition, 
            lightDirection, lightColor * lightIntensity, 
            lightAngleScale, lightAngleOffset);
    } 

    float3 finalColor = lit * BRDF;
    finalColor = finalColor * exposure;


    // トーンマップの適用
    float3 toneMapped = GT_Tonemap(finalColor);

    output.color = float4(toneMapped, baseColor.a);

    return output;
}