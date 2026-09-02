#include "Engine/Render/ShadowPass.h"

#include <cstddef>
#include <vector>

#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Model/VertexTypes.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Resource/ShaderLoader.h"
#include "Engine/Scene/Scene.h"

bool ShadowPass::Init(GraphicsDevice& device) {
    m_pDevice = &device;

    // ルートシグネチャの構築
    RootSignatureBuilder rsBuilder;
    // ルートシグネチャ構成
    // [b0] SceneConstants (Root CBV)
    // [b1] TransformConstants (Root CBV)
    rsBuilder
        .SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
        .AddCBV(0, 0, D3D12_SHADER_VISIBILITY_ALL,
            D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
        .AddCBV(1, 0, D3D12_SHADER_VISIBILITY_VERTEX,
            D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);

    if (!rsBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build root signature.\n");
        return false;
    }
    m_pRootSignature = rsBuilder.Get();

    // パイプラインステートの構築
    GraphicsPipelineBuilder psoBuilder;
    // シェーダーの読み込み（VSのみ）
    std::vector<std::byte> vsData;
    if (!LoadShader(L"shader/ShadowMapVS.cso", vsData)) {
        OutputDebugStringW(L"Failed to load shaders.\n");
        return false;
    }
    psoBuilder.SetRootSignature(m_pRootSignature.Get())
        .SetVertexShader(vsData.data(), vsData.size())
        .SetDepthMode(DepthMode::Default)
        .SetRenderTargetLayout(kShadowLayout)
        .SetInputLayout(StandardVertex::GetInputLayout())
        .SetDepthBias(0, 0.0f, 1.0f);  // シャドウマップ用の深度バイアス

    if (!psoBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build graphics pipeline state.\n");
        return false;
    }
    m_pPSO = psoBuilder.Get();

    return true;
}

void ShadowPass::Term() {
    m_pDevice = nullptr;

    m_pPSO.Reset();
    m_pRootSignature.Reset();
}

// 描画コマンドの記録
void ShadowPass::Draw(const ShadowPassBindings& passBindings, Scene& scene) {
    auto pCmdList = passBindings.pCmdList;

    // パイプライン設定
    pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    pCmdList->SetPipelineState(m_pPSO.Get());

    // [b0] SceneConstants (共通)
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Scene, passBindings.sceneCB);

    // PrimitiveTopologyの指定
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    scene.ForEachObject([&](GameObject& obj) {
        // [b1] TransformConstants (モデル単位)
        pCmdList->SetGraphicsRootConstantBufferView(RootParam::CBV_Transform,
            obj.GetTransformGPU(passBindings.frameIndex).GetGPUAddress());
        const auto model = scene.GetModel(obj.GetModelHandle());
        if (model == nullptr) return;
        const auto& meshes = model->GetMeshes();
        for (const auto& mesh : meshes) {
            // 頂点バッファ・インデックスバッファの設定
            auto vbv = mesh->GetVertexBufferView();
            auto ibv = mesh->GetIndexBufferView();
            pCmdList->IASetVertexBuffers(0, 1, &vbv);
            pCmdList->IASetIndexBuffer(&ibv);

            // 描画コマンドの発行
            pCmdList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
        }
    });
}
