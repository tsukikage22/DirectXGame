#include "Engine/Render/ScenePass.h"

#include <cassert>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Model/VertexTypes.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Resource/AssetPath.h"
#include "Engine/Scene/Scene.h"

bool ScenePass::Init(
    GraphicsDevice& device, ID3DBlob* vsBlob, ID3DBlob* psBlob) {
    m_pDevice = &device;
    // ルートシグネチャの生成
    {
        RootSignatureBuilder builder;

        // SRVのレンジを作成
        // [t0-t4, space0] PBR Textures (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> range;
        range.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> iesRange;
        iesRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t0, space2] Light StructuredBuffer (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> lightRange;
        lightRange.push_back(RootSignatureBuilder::CreateRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2,
            D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE));

        // ルートシグニチャ構成
        // [b0] SceneConstants (Root CBV)
        // [b1] TransformConstants (Root CBV)
        // [b2] Material Constants (Root CBV)
        // [b3] Display Constants (Root CBV)
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        // baseColor, metallic-roughness, normal, emissive, occlusion
        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        // [t0, space2] Light StructuredBuffer (Descriptor Table SRV)
        // [s0] Default Sampler (Static Sampler)
        // [s1] IES Profile Sampler (Static Sampler)
        builder
            .SetFlags(
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
            .AddCBV(0, 0, D3D12_SHADER_VISIBILITY_ALL,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(1, 0, D3D12_SHADER_VISIBILITY_VERTEX,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE)
            .AddCBV(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddCBV(3, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(range, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(iesRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(lightRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddStaticSampler(0)
            .AddStaticSampler(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // 垂直角は端で止める
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // 水平角は0-360°でループする
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        if (!builder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build root signature.\n");
            return false;
        }

        m_pRootSignature = builder.Get();
    }

    // パイプラインステートの生成
    {
        // グラフィックスパイプラインステートの設定
        GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.SetRootSignature(m_pRootSignature.Get())
            .SetVertexShader(vsBlob)
            .SetPixelShader(psBlob)
            .SetInputLayout(StandardVertex::GetInputLayout())
            .SetBlendState(BlendMode::Opaque)
            .SetRenderTargetLayout(kSceneLayout);

        if (!pipelineBuilder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build graphics pipeline state.\n");
            return false;
        }

        m_pPSO = pipelineBuilder.Get();
    }

    return true;
}

void ScenePass::Term() {
    m_pDevice = nullptr;

    m_pPSO.Reset();
    m_pRootSignature.Reset();
}

void ScenePass::Draw(const ScenePassBindings& passBindings, Scene& scene) {
    auto pCmdList = passBindings.pCmdList;

    // パイプライン設定
    pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    pCmdList->SetPipelineState(m_pPSO.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { passBindings.pCbvSrvUavHeap };
    pCmdList->SetDescriptorHeaps(1, ppHeaps);

    // [b0] SceneConstants (共通)
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Scene, passBindings.sceneCB);

    // [b3] DisplayConstants (共通)
    pCmdList->SetGraphicsRootConstantBufferView(
        RootParam::CBV_Display, passBindings.displayCB);

    // [t0, space1] IESプロファイルテクスチャ (共通)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_IESProfile, passBindings.iesSRV);

    // [t0, space2] Light StructuredBuffer (共通)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_Lights, passBindings.lightSRV);

    // PrimitiveTopologyの指定
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 全オブジェクトを描画
    scene.ForEachObject([&](GameObject& obj) {
        // [b1] TransformConstants (モデル単位)
        pCmdList->SetGraphicsRootConstantBufferView(RootParam::CBV_Transform,
            obj.GetTransformGPU(passBindings.frameIndex).GetGPUAddress());

        // 各メッシュを描画
        const auto model = scene.GetModel(obj.GetModelHandle());
        if (model == nullptr) return;
        const auto& meshes    = model->GetMeshes();
        const auto& materials = model->GetMaterials();
        for (auto& mesh : meshes) {
            // このメッシュが使うマテリアルを取得
            uint32_t materialID = mesh->GetMaterialID();

            // マテリアルが存在しない場合は描画しない
            if (materialID >= materials.size() ||
                materials[materialID] == nullptr) {
                assert(false && "Mesh has no valid material.");
                continue;
            }

            // マテリアルをバインド
            // [b2] MaterialConstants (マテリアル単位)
            pCmdList->SetGraphicsRootConstantBufferView(RootParam::CBV_Material,
                materials[materialID]->GetConstantBufferGPUAddress());

            // [t0-t4] PBR Textures
            pCmdList->SetGraphicsRootDescriptorTable(RootParam::SRV_Texture,
                materials[materialID]->GetSrvTableBaseGPUHandle());

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
