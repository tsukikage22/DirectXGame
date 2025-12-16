#include "Engine/RootSignatureBuilder.h"

#include "Engine/DxDebug.h"

// CBVのルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddCBV(UINT shaderRegister,
    UINT registerSpace, D3D12_SHADER_VISIBILITY visibility,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags) {
    D3D12_ROOT_PARAMETER1 param     = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.Descriptor.Flags          = flags;
    param.ShaderVisibility          = visibility;

    m_params.push_back(param);
    return *this;
}

// SRVのルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddSRV(UINT shaderRegister,
    UINT registerSpace, D3D12_SHADER_VISIBILITY visibility,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags) {
    D3D12_ROOT_PARAMETER1 param     = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.Descriptor.Flags          = flags;
    param.ShaderVisibility          = visibility;

    m_params.push_back(param);
    return *this;
}

// UAVのルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddUAV(UINT shaderRegister,
    UINT registerSpace, D3D12_SHADER_VISIBILITY visibility,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags) {
    D3D12_ROOT_PARAMETER1 param     = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_UAV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.Descriptor.Flags          = flags;
    param.ShaderVisibility          = visibility;

    m_params.push_back(param);
    return *this;
}

// ルート定数のルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddConstants(UINT num32BitValues,
    UINT shaderRegister, UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility) {
    D3D12_ROOT_PARAMETER1 param    = {};
    param.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues = num32BitValues;
    param.Constants.ShaderRegister = shaderRegister;
    param.Constants.RegisterSpace  = registerSpace;
    param.ShaderVisibility         = visibility;

    m_params.push_back(param);
    return *this;
}

// ディスクリプタレンジの初期化
D3D12_DESCRIPTOR_RANGE1 RootSignatureBuilder::CreateRange(
    D3D12_DESCRIPTOR_RANGE_TYPE type, UINT numDescriptors, UINT baseRegister,
    UINT registerSpace, D3D12_DESCRIPTOR_RANGE_FLAGS flags, UINT offset) {
    D3D12_DESCRIPTOR_RANGE1 range           = {};
    range.RangeType                         = type;
    range.NumDescriptors                    = numDescriptors;
    range.BaseShaderRegister                = baseRegister;
    range.RegisterSpace                     = registerSpace;
    range.Flags                             = flags;
    range.OffsetInDescriptorsFromTableStart = offset;

    return range;
}

// ディスクリプタテーブルのルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(
    const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges,
    D3D12_SHADER_VISIBILITY visibility) {
    // rangesの確認
    if (ranges.empty()) {
        return *this;
    }

    // ディスクリプタテーブル用のrangeデータを保存
    auto tableData    = std::make_unique<DescriptorTableData>();
    tableData->ranges = ranges;

    // ルートパラメータ設定
    D3D12_ROOT_PARAMETER1 param = {};
    param.ParameterType         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges =
        static_cast<UINT>(tableData->ranges.size());
    param.DescriptorTable.pDescriptorRanges = tableData->ranges.data();
    param.ShaderVisibility                  = visibility;

    m_tableData.push_back(std::move(tableData));
    m_params.push_back(param);
    return *this;
}

// スタティックサンプラのルートパラメータ定義
RootSignatureBuilder& RootSignatureBuilder::AddStaticSampler(
    UINT shaderRegister, D3D12_FILTER filter,
    D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV,
    D3D12_TEXTURE_ADDRESS_MODE addressW, UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility) {
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = filter;
    sampler.AddressU                  = addressU;
    sampler.AddressV                  = addressV;
    sampler.AddressW                  = addressW;
    sampler.MipLODBias                = 0.0f;
    sampler.MaxAnisotropy             = 1;
    sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD           = 0.0f;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = shaderRegister;
    sampler.RegisterSpace    = registerSpace;
    sampler.ShaderVisibility = visibility;

    m_samplers.push_back(sampler);
    return *this;
}

bool RootSignatureBuilder::Build(ID3D12Device* pDevice) {
    // ディスクリプタの作成
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
    desc.Version                             = D3D_ROOT_SIGNATURE_VERSION_1_1;
    desc.Desc_1_1.NumParameters = static_cast<UINT>(m_params.size());
    desc.Desc_1_1.pParameters   = m_params.empty() ? nullptr : m_params.data();
    desc.Desc_1_1.NumStaticSamplers = static_cast<UINT>(m_samplers.size());
    desc.Desc_1_1.pStaticSamplers   = m_samplers.data();
    desc.Desc_1_1.Flags             = m_flags;

    // シリアライズ
    engine::ComPtr<ID3DBlob> pBlob = nullptr;
    engine::ComPtr<ID3DBlob> pErr  = nullptr;

    CHECK_HR(pDevice, D3D12SerializeVersionedRootSignature(
                          &desc, pBlob.GetAddressOf(), pErr.GetAddressOf()));

    // ルートシグニチャの生成
    CHECK_HR(pDevice, pDevice->CreateRootSignature(0, pBlob->GetBufferPointer(),
                          pBlob->GetBufferSize(),
                          IID_PPV_ARGS(m_pRootSignature.GetAddressOf())));

    return true;
}

// リセット
void RootSignatureBuilder::Reset() {
    m_params.clear();
    m_samplers.clear();
    m_tableData.clear();
    m_flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
}

// 取得
ID3D12RootSignature* RootSignatureBuilder::Get() const {
    return m_pRootSignature.Get();
}