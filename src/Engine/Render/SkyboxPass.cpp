
#include "Engine/Render/SkyboxPass.h"

#include <cstddef>
#include <vector>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Resource/ShaderLoader.h"

bool SkyboxPass::Init(GraphicsDevice& device) {
    m_pDevice = &device;

    // ルートシグネチャの構築
    RootSignatureBuilder rsBuilder;
    std::vector<D3D12_DESCRIPTOR_RANGE1> range;
    range.push_back(
        RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
            0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));
    // ルートシグネチャ構成
    // [b0] SceneConstants (Root CBV)
    // [b3] Display Constants (Root CBV)
    // [t0, space0] Skybox Texture (Descriptor Table SRV)
    // [s0] Sampler: Linear sampler
    rsBuilder
        .AddCBV(0, 0, D3D12_SHADER_VISIBILITY_ALL,
            D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
        .AddCBV(3, 0, D3D12_SHADER_VISIBILITY_ALL,
            D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
        .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
        .AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    if (!rsBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build root signature.\n");
        return false;
    }
    m_pRootSignature = rsBuilder.Get();

    // パイプラインステートの構築
    GraphicsPipelineBuilder psoBuilder;
    // シェーダーの読み込み
    std::vector<std::byte> vsData;
    std::vector<std::byte> psData;
    if (!LoadShader(L"shader/SkyboxVS.cso", vsData) ||
        !LoadShader(L"shader/SkyboxPS.cso", psData)) {
        OutputDebugStringW(L"Failed to load shaders.\n");
        return false;
    }
    psoBuilder.SetRootSignature(m_pRootSignature.Get())
        .SetVertexShader(vsData.data(), vsData.size())
        .SetPixelShader(psData.data(), psData.size())
        .SetBlendState(BlendMode::Opaque)
        .SetDepthMode(DepthMode::ReadOnly)
        .SetRenderTargetLayout(kSceneLayout);

    if (!psoBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build graphics pipeline state.\n");
        return false;
    }
    m_pPSO = psoBuilder.Get();

    return true;
}

void SkyboxPass::Term() {
    m_pDevice = nullptr;

    m_pPSO.Reset();
    m_pRootSignature.Reset();
}

// 描画コマンドの記録
void SkyboxPass::Draw(const SkyboxPassBindings& passBindings) {
    auto pCmdList = passBindings.pCmdList;

    // パイプライン設定
    pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    pCmdList->SetPipelineState(m_pPSO.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { passBindings.pCbvSrvUavHeap };
    pCmdList->SetDescriptorHeaps(1, ppHeaps);

    // [b0] SceneConstants (共通)
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Scene, passBindings.sceneCB);

    // [b3] Display Constants (共通)
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Display, passBindings.displayCB);

    // [t0, space0] Skybox Texture (Descriptor Table SRV)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_Skybox, passBindings.skyboxSRV);

    // 描画
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmdList->DrawInstanced(3, 1, 0, 0);  // フルスクリーン三角形を描画
}
