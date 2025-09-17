/// @file Pipeline.h
/// @brief ルートシグニチャとPSO

#include <d3d12.h>

#include <vector>

#include "Engine/ComPtr.h"

class GraphicsPipelineBuilder {
public:
    GraphicsPipelineBuilder()  = default;
    ~GraphicsPipelineBuilder() = default;

    /// @brief
    /// デフォルトのパイプライン設定を行う(ラスタライザステートとブレンドステート)
    /// @return
    GraphicsPipelineBuilder& SetDefault();

    /// @brief ルートシグニチャを設定する
    /// @param pRootSignature
    /// @return
    GraphicsPipelineBuilder& SetRootSignature(
        ID3D12RootSignature* pRootSignature);

    /// @brief 頂点シェーダーを設定する
    /// @param pShaderBytecode
    /// @param bytecodeLength
    /// @return
    GraphicsPipelineBuilder& SetVertexShader(ID3DBlob* pVSBlob);

    /// @brief ピクセルシェーダーを設定する
    /// @param pShaderBytecode
    /// @param bytecodeLength
    /// @return
    GraphicsPipelineBuilder& SetPixelShader(ID3DBlob* pPSBlob);

    /// @brief 入力レイアウトを設定する
    /// @param inputLayout
    GraphicsPipelineBuilder& SetInputLayout(
        const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements);

    GraphicsPipelineBuilder& SetRTVFormat(DXGI_FORMAT format);
    GraphicsPipelineBuilder& SetDSVFormat(DXGI_FORMAT format);

    bool Build(ID3D12Device* pDevice);

    /// @brief パイプラインステートの取得
    /// @return パイプラインステート
    ID3D12PipelineState* Get() const { return m_pPipelineState.Get(); }

private:
    engine::ComPtr<ID3D12PipelineState> m_pPipelineState;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSOdesc = {};

    // コピー・ムーブ禁止
    GraphicsPipelineBuilder(const GraphicsPipelineBuilder&)            = delete;
    GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder&&)                 = delete;
    GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder&&)      = delete;
};