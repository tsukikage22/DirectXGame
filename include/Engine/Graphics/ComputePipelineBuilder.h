/// @file ComputePipelineBuilder.h
/// @brief コンピュートシェーダー用のPSOを作成するためのビルダークラス

#pragma once

#include <d3d12.h>

#include <cstddef>

#include "Engine/Core/ComPtr.h"

class ComputePipelineBuilder
{
public:
    ComputePipelineBuilder()
    {
        SetDefault();
    };
    ~ComputePipelineBuilder() = default;

    /// @brief ルートシグニチャを設定する
    /// @param pRootSignature
    /// @return
    ComputePipelineBuilder& SetRootSignature(ID3D12RootSignature* pRootSignature);

    /// @brief コンピュートシェーダーを設定する
    /// @param pShaderBytecode シェーダーのバイトコード
    /// @param bytecodeLength バイトコードの長さ
    /// @note バイトコードはBuildまで呼び出し側が保持する必要がある
    /// @return
    ComputePipelineBuilder& SetComputeShader(const std::byte* pShaderBytecode, std::size_t bytecodeLength);

    /// @brief PSOの作成
    /// @param pDevice デバイス
    /// @return 成功したかどうか
    bool Build(ID3D12Device* pDevice);

    /// @brief PSOの取得
    /// @return 作成されたPSO
    ID3D12PipelineState* Get() const
    {
        return m_pPSO.Get();
    }

private:
    /// @brief デフォルトのパイプライン設定を行う
    void SetDefault();

    D3D12_COMPUTE_PIPELINE_STATE_DESC m_PSOdesc = {};
    engine::ComPtr<ID3D12PipelineState> m_pPSO;

    // コピー・ムーブ禁止
    ComputePipelineBuilder(const ComputePipelineBuilder&)            = delete;
    ComputePipelineBuilder& operator=(const ComputePipelineBuilder&) = delete;
    ComputePipelineBuilder(ComputePipelineBuilder&&)                 = delete;
    ComputePipelineBuilder& operator=(ComputePipelineBuilder&&)      = delete;
};
