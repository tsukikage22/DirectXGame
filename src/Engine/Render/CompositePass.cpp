
#include "Engine/Render/CompositePass.h"

#include <cstddef>
#include <vector>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Resource/ShaderLoader.h"

bool CompositePass::Init(GraphicsDevice& device) {
    m_pDevice = &device;
    // ルートシグネチャの生成
    // ルートシグネチャの構成
    // [b3] Display Constants (Root CBV)
    // [t0] UI Texture (Descriptor Table SRV)
    auto rsBuilder = RootSignatureBuilder{};
    std::vector<D3D12_DESCRIPTOR_RANGE1> range;
    range.push_back(rsBuilder.CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
        0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE));
    rsBuilder
        .AddCBV(3, 0, D3D12_SHADER_VISIBILITY_ALL,
            D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
        .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL);

    if (!rsBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build UI root signature.\n");
        return false;
    }
    m_pRootSignature = rsBuilder.Get();

    // シェーダーの読み込み
    std::vector<std::byte> vsData;
    std::vector<std::byte> psData;
    if (!LoadShader(L"shader/UI_VS.cso", vsData) ||
        !LoadShader(L"shader/UI_PS.cso", psData)) {
        OutputDebugStringW(L"Failed to load shaders.\n");
        return false;
    }

    // パイプラインステートの生成
    auto psoBuilder = GraphicsPipelineBuilder{};
    psoBuilder.SetRootSignature(m_pRootSignature.Get())
        .SetVertexShader(vsData.data(), vsData.size())
        .SetPixelShader(psData.data(), psData.size())
        .SetBlendState(BlendMode::PremultipliedAlpha)
        .SetDepthMode(DepthMode::Disabled)
        .SetRenderTargetLayout(kCompositeLayout);

    if (!psoBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build UI pipeline state.\n");
        return false;
    }
    m_pPSO = psoBuilder.Get();

    return true;
}

void CompositePass::Term() {
    m_pDevice = nullptr;

    m_pPSO.Reset();
    m_pRootSignature.Reset();
}

void CompositePass::Draw(const CompositePassBindings& passBindings) {
    auto pCmdList = passBindings.pCmdList;

    // ルートシグネチャとパイプラインステートの設定
    pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    pCmdList->SetPipelineState(m_pPSO.Get());

    // CBVとしてDisplayConstantsを設定
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Display, passBindings.displayCB);

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* pHeaps[] = { passBindings.pCbvSrvUavHeap };
    pCmdList->SetDescriptorHeaps(1, pHeaps);
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_UI, passBindings.uiSRV);

    // 描画
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmdList->DrawInstanced(3, 1, 0, 0);  // フルスクリーン三角形を描画
}
