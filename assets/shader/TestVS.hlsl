//===========================================
// VS Input structure
//===========================================
// 入力頂点データ（Engine側のVertexBufferの構造（StandardVertex）に対応）
struct VSInput{
    float3 position : POSITION;     // 頂点座標
    float3 normal   : NORMAL;       // 頂点法線
    float2 texCoord : TEXCOORD;     // テクスチャ座標
    float3 tangent  : TANGENT;      // 接線ベクトル
}

//===========================================
// VS Output structure
//===========================================
// ピクセルシェーダーへ渡すデータ
struct VSOutput{
    float4 position : SV_POSITION;  // 変換後頂点座標
    float2 texCoord : TEXCOORD;     // テクスチャ座標
};

//===========================================
// constants buffer
//===========================================
// [b0] シーン定数（View, Projection行列）
cbuffer Transform : register(b0) {
    float4x4 view   : packoffset(c4); // ビュー行列
    float4x4 proj   : packoffset(c8); // プロジェクション行列
    float3 cameraPos;   // カメラ位置（ワールド座標系）
};

// [b1] ワールド変換行列
cbuffer SceneConstants : register(b1) {
    float4x4 world;
    float4x4 worldInv;
};

VSOutPut main(VSInput input) {
    VSOutput output;

    // 1. ローカル座標 -> ワールド座標変換
    float4 worldPos = mul(float4(input.position, 1.0f), world);

    // 2. ワールド座標 -> ビュー座標変換
    float4 viewPos = mul(worldPos, view);

    // 3. ビュー座標 -> 射影変換
    output.position = mul(viewPos, proj);

    // 法線変換
    output.normal = mul(input.normal, (float3x3)world);

    // UV座標の受け渡し
    output.texCoord = input.texCoord;

    return output;
}