/// @file GraphicsPipelineBuilder.h
/// @brief PSOを作成するためのビルダークラス

#pragma once

#include <d3d12.h>

#include <cstddef>
#include <vector>

#include "Engine/Core/ComPtr.h"
#include "Engine/Graphics/RenderTargetLayout.h"

enum class BlendMode {
    Opaque,              // 不透明（既定）
    AlphaBlend,          // ストレートアルファ合成
    PremultipliedAlpha,  // 乗算アルファ合成
    Additive,            // 加算合成
};

class GraphicsPipelineBuilder {
public:
    GraphicsPipelineBuilder();
    ~GraphicsPipelineBuilder() = default;

    /// @brief ルートシグニチャを設定する
    /// @param pRootSignature
    /// @return
    GraphicsPipelineBuilder& SetRootSignature(
        ID3D12RootSignature* pRootSignature);

    /// @brief 頂点シェーダーを設定する
    /// @param pShaderBytecode シェーダーのバイトコード
    /// @param bytecodeLength バイトコードの長さ
    /// @note バイトコードはBuildまで呼び出し側が保持する必要がある
    /// @return
    GraphicsPipelineBuilder& SetVertexShader(
        const std::byte* pShaderBytecode, std::size_t bytecodeLength);

    /// @brief ピクセルシェーダーを設定する
    /// @param pShaderBytecode シェーダーのバイトコード
    /// @param bytecodeLength バイトコードの長さ
    /// @note バイトコードはBuildまで呼び出し側が保持する必要がある
    /// @return
    GraphicsPipelineBuilder& SetPixelShader(
        const std::byte* pShaderBytecode, std::size_t bytecodeLength);

    /// @brief 入力レイアウトを設定する
    /// @param inputLayout
    GraphicsPipelineBuilder& SetInputLayout(
        const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements);

    /// @brief 全RTに指定したBlendModeを設定する
    /// @param blendMode ブレンドモード
    GraphicsPipelineBuilder& SetBlendState(BlendMode blendMode);

    /// @brief RTLayout定数からPSOの設定を行う
    /// @param layout レンダーターゲットのレイアウト
    GraphicsPipelineBuilder& SetRenderTargetLayout(
        const RenderTargetLayout& layout);

    bool Build(ID3D12Device* pDevice);

    /// @brief パイプラインステートの取得
    /// @return パイプラインステート
    ID3D12PipelineState* Get() const { return m_pPipelineState.Get(); }

private:
    /// @brief
    /// デフォルトのパイプライン設定を行う(ラスタライザステートとブレンドステート)
    /// @return
    void SetDefault();

    engine::ComPtr<ID3D12PipelineState> m_pPipelineState;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSOdesc = {};

    // コピー・ムーブ禁止
    GraphicsPipelineBuilder(const GraphicsPipelineBuilder&)            = delete;
    GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder&&)                 = delete;
    GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder&&)      = delete;
};

/// @brief ブレンド設定のプリセットを作成する
/// @param blendMode ブレンドモード
/// @return 設定されたD3D12_RENDER_TARGET_BLEND_DESC
constexpr D3D12_RENDER_TARGET_BLEND_DESC MakeRenderTargetBlendDesc(
    BlendMode blendMode) {
    D3D12_RENDER_TARGET_BLEND_DESC desc = {};
    desc.BlendEnable                    = (blendMode != BlendMode::Opaque);
    desc.LogicOpEnable                  = FALSE;
    desc.SrcBlend                       = D3D12_BLEND_ONE;
    desc.DestBlend                      = D3D12_BLEND_ZERO;
    desc.BlendOp                        = D3D12_BLEND_OP_ADD;
    desc.SrcBlendAlpha                  = D3D12_BLEND_ONE;
    desc.DestBlendAlpha                 = D3D12_BLEND_ZERO;
    desc.BlendOpAlpha                   = D3D12_BLEND_OP_ADD;
    desc.LogicOp                        = D3D12_LOGIC_OP_NOOP;
    desc.RenderTargetWriteMask          = D3D12_COLOR_WRITE_ENABLE_ALL;

    switch (blendMode) {
        case BlendMode::AlphaBlend:
            desc.SrcBlend      = D3D12_BLEND_SRC_ALPHA;
            desc.DestBlend     = D3D12_BLEND_INV_SRC_ALPHA;
            desc.SrcBlendAlpha = D3D12_BLEND_ONE;
            desc.DestBlendAlpha =
                D3D12_BLEND_INV_SRC_ALPHA;  // アルファ値も考慮する場合
            break;

        case BlendMode::PremultipliedAlpha:
            desc.SrcBlend      = D3D12_BLEND_ONE;
            desc.DestBlend     = D3D12_BLEND_INV_SRC_ALPHA;
            desc.SrcBlendAlpha = D3D12_BLEND_ONE;
            desc.DestBlendAlpha =
                D3D12_BLEND_INV_SRC_ALPHA;  // アルファ値も考慮する場合
            break;

        case BlendMode::Additive:
            desc.SrcBlend       = D3D12_BLEND_SRC_ALPHA;
            desc.DestBlend      = D3D12_BLEND_ONE;
            desc.SrcBlendAlpha  = D3D12_BLEND_ONE;
            desc.DestBlendAlpha = D3D12_BLEND_ONE;  // アルファ値も考慮する場合
            break;

        default:
            break;
    }

    return desc;
}