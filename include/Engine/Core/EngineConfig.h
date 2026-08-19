/// @file EngineConfig.h
/// @brief 共有定数

#pragma once

#include <cstdint>

namespace config {
inline constexpr DXGI_FORMAT kBackBufferFormat =
    DXGI_FORMAT_R16G16B16A16_FLOAT;  // バックバッファのフォーマット
inline constexpr DXGI_FORMAT kDepthBufferFormat =
    DXGI_FORMAT_D32_FLOAT;  // 深度バッファのフォーマット
inline constexpr DXGI_FORMAT kUIBufferFormat =
    DXGI_FORMAT_R8G8B8A8_UNORM;  // UI用バッファのフォーマット

inline constexpr uint32_t kFrameCount = 2;      // フレームリソースの数
inline constexpr uint32_t kMaxObjects = 10000;  // 最大オブジェクト数
inline constexpr uint32_t kMaxLights  = 64;     // 最大ライト数

// 1マテリアル当たりの見積もり
inline constexpr uint32_t kMaxMaterials       = 2560;  // 最大マテリアル数
inline constexpr uint32_t kTexturePerMaterial = 5;
inline constexpr uint32_t kMiscSrvCbvReserve  = 256;  // IES/IBLなど

// CBV/SRV/UAVヒープの最大数
inline constexpr uint32_t kCbvSrvUavCapacity =
    kMaxObjects * kFrameCount                    // Transform CBV
    + kMaxMaterials * (1 + kTexturePerMaterial)  // Material CBV + PBRテクスチャ
    + kFrameCount                                // Scene CBV
    + kFrameCount                                // Light StructuredBuffer
    + kMiscSrvCbvReserve;                        // IES/IBLなど

inline constexpr uint32_t kSamplerCapacity = 256;              // <= 2048
inline constexpr uint32_t kRtvCapacity     = kFrameCount + 8;  // バックバッファ
inline constexpr uint32_t kDsvCapacity     = 1 + 4;  // メイン深度 + 余白
inline constexpr uint32_t kAssetSrvCapacity = 2048;  // アセット用SRVの最大数

inline constexpr float kHDRPaperWhiteNits = 200.0f;  // HDRの紙白輝度（nits）
inline constexpr float kSDRPaperWhiteNits = 80.0f;   // SDRの紙白輝度（nits）
}  // namespace config
