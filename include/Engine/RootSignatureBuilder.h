/// @file   RootSignatureBuilder.h
/// @brief  ルートシグネチャを生成するためのビルダークラス
#pragma once

#include <d3d12.h>

#include <memory>
#include <vector>

#include "Engine/ComPtr.h"

class RootSignatureBuilder {
public:
    RootSignatureBuilder()  = default;
    ~RootSignatureBuilder() = default;

    /// @brief 定数バッファビュー(CBV)のルートパラメータ定義
    /// @param shaderRegister シェーダーレジスタ番号
    /// @param registerSpace レジスタ空間
    /// @param visibility シェーダー可視性
    /// @param flags ルートディスクリプタフラグ
    /// @return 自身の参照
    RootSignatureBuilder& AddCBV(UINT shaderRegister, UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags  = D3D12_ROOT_DESCRIPTOR_FLAG_NONE);

    /// @brief シェーダーリソースビュー(SRV)のルートパラメータ定義
    /// @param shaderRegister
    /// @param registerSpace
    /// @param visibility
    /// @param flags
    /// @return
    RootSignatureBuilder& AddSRV(UINT shaderRegister, UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags  = D3D12_ROOT_DESCRIPTOR_FLAG_NONE);

    /// @brief シェーダーリソースビュー(SRV)のルートパラメータ定義
    /// @param shaderRegister
    /// @param registerSpace
    /// @param visibility
    /// @param flags
    /// @return
    RootSignatureBuilder& AddUAV(UINT shaderRegister, UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags  = D3D12_ROOT_DESCRIPTOR_FLAG_NONE);

    /// @brief ルート定数のルートパラメータ定義
    /// @param num32BitValues
    /// @param shaderRegister
    /// @param registerSpace
    /// @param visibility
    /// @return
    RootSignatureBuilder& AddConstants(UINT num32BitValues, UINT shaderRegister,
        UINT registerSpace                 = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    /// @brief ディスクリプタレンジの定義
    /// @param type
    /// @param numDescriptors
    /// @param baseRegister
    /// @param registerSpace
    /// @param flags
    /// @param offset
    /// @return
    static D3D12_DESCRIPTOR_RANGE1 CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE type,
        UINT numDescriptors, UINT baseRegister, UINT registerSpace = 0,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        UINT offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

    /// @brief ディスクリプタテーブルのルートパラメータ定義
    /// @param ranges
    /// @param visibility
    /// @return
    RootSignatureBuilder& AddDescriptorTable(
        const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    /// @brief スタティックサンプラーのルートパラメータ定義
    /// @param shaderRegister
    /// @param filter
    /// @param addressU
    /// @param addressV
    /// @param addressW
    /// @param registerSpace
    /// @param visibility
    /// @return
    RootSignatureBuilder& AddStaticSampler(UINT shaderRegister,
        D3D12_FILTER filter                 = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE addressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        UINT registerSpace                  = 0,
        D3D12_SHADER_VISIBILITY visibility  = D3D12_SHADER_VISIBILITY_ALL);

    /// @brief ルートシグネチャのフラグ設定
    /// @param flags
    /// @return
    RootSignatureBuilder& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) {
        m_flags = flags;
        return *this;
    }

    bool Build(ID3D12Device* pDevice, ID3D12RootSignature** ppRootSignature);

    void Reset();

private:
    /// @brief ディスクリプタテーブル用のrangeデータ
    struct DescriptorTableData {
        std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;
    };

    std::vector<D3D12_ROOT_PARAMETER1> m_params;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
    std::vector<std::unique_ptr<DescriptorTableData>> m_tableData;
    D3D12_ROOT_SIGNATURE_FLAGS m_flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    // コピー・ムーブ禁止
    RootSignatureBuilder(const RootSignatureBuilder&)            = delete;
    RootSignatureBuilder& operator=(const RootSignatureBuilder&) = delete;
    RootSignatureBuilder(RootSignatureBuilder&&)                 = delete;
    RootSignatureBuilder& operator=(RootSignatureBuilder&&)      = delete;
};