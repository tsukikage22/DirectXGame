#include "Engine/GraphicsPipelineBuilder.h"

// デフォルトのパイプライン設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDefault() {
    // ラスタライザステートの設定
    D3D12_RASTERIZER_DESC RSdesc = {};
    RSdesc.FillMode              = D3D12_FILL_MODE_SOLID;
    RSdesc.CullMode              = D3D12_CULL_MODE_NONE;
    RSdesc.FrontCounterClockwise = FALSE;
    RSdesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
    RSdesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    RSdesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    RSdesc.DepthClipEnable       = TRUE;
    RSdesc.MultisampleEnable     = FALSE;
    RSdesc.AntialiasedLineEnable = FALSE;
    RSdesc.ForcedSampleCount     = 0;
    RSdesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ブレンドステートの設定
    D3D12_RENDER_TARGET_BLEND_DESC RTVdesc = {};
    RTVdesc.BlendEnable                    = FALSE;
    RTVdesc.LogicOpEnable                  = FALSE;
    RTVdesc.SrcBlend                       = D3D12_BLEND_ONE;
    RTVdesc.DestBlend                      = D3D12_BLEND_ZERO;
    RTVdesc.BlendOp                        = D3D12_BLEND_OP_ADD;
    RTVdesc.SrcBlendAlpha                  = D3D12_BLEND_ONE;
    RTVdesc.DestBlendAlpha                 = D3D12_BLEND_ZERO;
    RTVdesc.BlendOpAlpha                   = D3D12_BLEND_OP_ADD;
    RTVdesc.LogicOp                        = D3D12_LOGIC_OP_NOOP;
    RTVdesc.RenderTargetWriteMask          = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_BLEND_DESC BSdesc       = {};
    BSdesc.AlphaToCoverageEnable  = FALSE;
    BSdesc.IndependentBlendEnable = FALSE;
    for (auto i = 0u; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
        BSdesc.RenderTarget[i] = RTVdesc;
    }

    // 設定を反映
    m_PSOdesc.RasterizerState = RSdesc;
    m_PSOdesc.BlendState      = BSdesc;

    // その他の設定
    m_PSOdesc.SampleMask            = UINT_MAX;
    m_PSOdesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_PSOdesc.NumRenderTargets      = 1;
    m_PSOdesc.SampleDesc.Count      = 1;
    m_PSOdesc.SampleDesc.Quality    = 0;

    return *this;
}

// ルートシグニチャの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRootSignature(
    ID3D12RootSignature* pRootSignature) {
    m_PSOdesc.pRootSignature = pRootSignature;
    return *this;
}

// 頂点シェーダーの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexShader(
    ID3DBlob* pVSBlob) {
    m_PSOdesc.VS.pShaderBytecode = pVSBlob->GetBufferPointer();
    m_PSOdesc.VS.BytecodeLength  = pVSBlob->GetBufferSize();
    return *this;
}

// ピクセルシェーダーの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPixelShader(
    ID3DBlob* pPSBlob) {
    m_PSOdesc.PS.pShaderBytecode = pPSBlob->GetBufferPointer();
    m_PSOdesc.PS.BytecodeLength  = pPSBlob->GetBufferSize();
    return *this;
}

// 入力レイアウトの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetInputLayout(
    const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements) {
    m_InputElements = elements;

    m_PSOdesc.InputLayout.pInputElementDescs = m_InputElements.data();
    m_PSOdesc.InputLayout.NumElements =
        static_cast<UINT>(m_InputElements.size());
    return *this;
}

// RTVフォーマットの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRTVFormat(
    DXGI_FORMAT format) {
    m_PSOdesc.RTVFormats[0] = format;
    return *this;
}

// DSVフォーマットの設定
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDSVFormat(
    DXGI_FORMAT format) {
    m_PSOdesc.DSVFormat = format;
    return *this;
}

// パイプラインステートの生成
bool GraphicsPipelineBuilder::Build(
    ID3D12Device* pDevice, ID3D12PipelineState** ppPipelineState) {
    // 頂点シェーダーとピクセルシェーダーが設定されているか
    if (m_PSOdesc.VS.pShaderBytecode == nullptr ||
        m_PSOdesc.PS.pShaderBytecode == nullptr) {
        return false;
    }

    // ルートシグニチャが設定されているか
    if (m_PSOdesc.pRootSignature == nullptr) {
        return false;
    }

    // レンダーターゲットのフォーマットが設定されているか
    if (m_PSOdesc.NumRenderTargets == 0) {
        return false;
    }

    // パイプラインステートの生成
    HRESULT hr = pDevice->CreateGraphicsPipelineState(
        &m_PSOdesc, IID_PPV_ARGS(m_pPipelineState.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    if (ppPipelineState) {
        *ppPipelineState = m_pPipelineState.Get();
        (*ppPipelineState)->AddRef();
    }

    return true;
}