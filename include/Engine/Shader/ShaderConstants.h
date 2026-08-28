/// @file ShaderConstants.h
/// @brief シェーダーに渡す定数バッファの構造体
#pragma once

#include <directxmath.h>

#include <cstdint>

namespace shader {

/// @brief デバッグビューの種類
enum class DebugView : uint32_t {
    FinalColor       = 0,  // 最終結果
    BaseColor        = 1,  // baseColor
    Normal           = 2,  // 法線
    Roughness        = 3,  // roughness
    Metallic         = 4,  // metallic
    AO               = 5,  // Ambient Occlusion
    WhiteFurnaceTest = 6,
    size             = 7
};

// デバッグビューの名前
constexpr const char* kDebugViewNames[] = { "Final Color", "Base Color",
    "Normal", "Roughness", "Metallic", "AO", "White Furnace Test" };

// DebugViewとNamesの整合性チェック
static_assert(
    std::size(kDebugViewNames) == static_cast<size_t>(DebugView::size),
    "Mismatch between debug view names and enum size");

//================================
// フレーム毎に更新する定数
//================================

/// @brief シーン全体に関わる定数（フレーム毎更新）
struct SceneConstants {
    DirectX::XMFLOAT4X4 view;         // ビュー行列
    DirectX::XMFLOAT4X4 projection;   // 射影行列
    DirectX::XMFLOAT4X4 invViewProj;  // NDCからワールド座標に変換するための行列
    DirectX::XMFLOAT3 cameraPosition;  // カメラ位置
    float time;                        // ゲーム時間
    float exposure;                    // 露出調整値
    uint32_t lightCount;               // ライトの数
    uint32_t debugView;                // 表示モード DebugViewの値を格納
    float envIntensity;                // 環境マップの輝度スケール係数
    uint32_t prefilteredMipCount;      // prefilteredのmip数
    float _padding[3];                 // 16バイトアラインメント用
};
static_assert(sizeof(SceneConstants) % 16 == 0, "Must be 16-byte aligned");

/// @brief ライティング計算に関わる定数
struct LightConstants {
    DirectX::XMFLOAT3 position;  // ライトの位置
    uint32_t
        type;  // ライトの種類（0: ディレクショナルライト, 1: ポイントライト, 2:
               // スポットライト, 3: フォトメトリックライト）

    DirectX::XMFLOAT3 forward;  // ライトの方向
    float invSqrRadius;         // 影響半径の二乗の逆数（計算の打ち切りに使う）

    DirectX::XMFLOAT3 color;  // ライトの色
    float intensity;          // ライトの強度（光度[cd]，平行光源のみ照度[lx]）

    float angleScale;   // スポットライトの角度減衰係数
    float angleOffset;  // スポットライトの角度オフセット
    uint32_t iesIndex;  // IESプロファイルのインデックス
    float _padding[1];  // 16バイトアラインメント用
};
static_assert(
    sizeof(LightConstants) == 64, "Must be matched with shader struct size");

//================================
// オブジェクト毎に更新する定数
//================================

/// @brief ワールド行列（オブジェクトごと更新）
struct TransformConstants {
    DirectX::XMFLOAT4X4 world;         // ワールド行列
    DirectX::XMFLOAT4X4 worldInverse;  // ワールド逆行列
};
static_assert(sizeof(TransformConstants) % 16 == 0, "Must be 16-byte aligned");

//================================
// マテリアル毎に更新する定数
//================================

/// @brief PBRパラメータ
struct MaterialConstants {
    DirectX::XMFLOAT4 baseColor;
    float metallic;
    float roughness;
    float _padding0;  // 16バイトアラインメント用
    float _padding1;  // 16バイトアラインメント用
    DirectX::XMFLOAT3 emissive;
    float occlusion;
};
static_assert(sizeof(MaterialConstants) % 16 == 0, "Must be 16-byte aligned");

//================================
// ポストプロセス・表示用定数
//================================

/// @brief ディスプレイ情報，ウィンドウ移動時に更新
struct DisplayConstants {
    float maxLuminance;           // 最大輝度
    float minLuminance;           // 最小輝度
    float paperWhiteNits;         // SDRの白の明るさ
    float maxFullFrameLuminance;  // 全白時の最大輝度
};
}  // namespace shader