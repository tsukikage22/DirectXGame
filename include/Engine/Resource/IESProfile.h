#pragma once

#include <d3d12.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Resource/TextureResource.h"

// 前方宣言
class DescriptorPool;

//-----------------------------------------------
// Light Source Structure
//-----------------------------------------------
struct IESProfileData {
    int lampCount;  // ランプ数

    float lumensPerLamp;      // ランプあたりの光束
    float candelaMultiplier;  // 乗算係数

    int photometricType;  // 測定座標系
    int unitType;         // 単位

    float shapeWidth;   // 形状横幅
    float shapeLength;  // 形状奥行
    float shapeHeight;  // 形状高さ

    float ballastFactor;  // 安定器光出力係数
    float inputWattage;   // 入力ワット数

    std::vector<float> anglesV;  // 垂直角
    std::vector<float> anglesH;  // 水平角
    std::vector<float> candela;  // カンデラ値

    float maxCandela;  // 最大カンデラ値
    float aveCandela;  // 平均カンデラ値
};

class IESProfile {
public:
    IESProfile();
    ~IESProfile();

    /// @brief 初期化処理
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool);

    /// @brief 終了処理
    void Term();

    /// @brief IESProfileを読み込み，テクスチャを追加する
    /// @return 作成したテクスチャのインデックス（Lightに渡す）
    std::optional<uint32_t> CreateIESTexture(
        const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch);

    //------------------------------------------------
    // アクセサ
    //------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const;
    uint32_t GetCount() const { return m_count; }

private:
    constexpr static uint32_t kMaxIESProfiles = 8;  // 最大IESプロファイル数
    constexpr static uint32_t kWidth  = 256;  // テクスチャの幅 θ（垂直角）
    constexpr static uint32_t kHeight = 128;  // テクスチャの高さ φ（水平角）

    TextureResource m_textureArray;  // IESプロファイルのテクスチャ
    DescriptorAllocation m_srv;      // SRVディスクリプタ
    DescriptorPool* m_pPoolSRV;      // ディスクリプタプール
    ID3D12Device* m_pDevice;         // デバイス

    uint32_t m_count;  // 読み込まれたIESプロファイルの数

    // コピー禁止
    IESProfile(const IESProfile&)            = delete;
    IESProfile& operator=(const IESProfile&) = delete;
};