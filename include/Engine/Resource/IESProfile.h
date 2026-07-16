#pragma once

#include <d3d12.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <filesystem>
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
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPool,
        std::filesystem::path path, DirectX::ResourceUploadBatch& batch);

    /// @brief 終了処理
    void Term();

    //------------------------------------------------
    // アクセサ
    //------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const;

private:
    TextureResource m_texture;   // IESプロファイルのテクスチャ
    DescriptorAllocation m_srv;  // SRVディスクリプタ
    DescriptorPool* m_pPoolSRV;  // ディスクリプタプール

    // コピー禁止
    IESProfile(const IESProfile&)            = delete;
    IESProfile& operator=(const IESProfile&) = delete;
};