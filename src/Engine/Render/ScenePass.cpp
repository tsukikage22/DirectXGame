#include "Engine/Render/ScenePass.h"

#include <cassert>
#include <cstddef>
#include <vector>

#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Model/VertexTypes.h"
#include "Engine/Render/PassBindings.h"
#include "Engine/Resource/ShaderLoader.h"
#include "Engine/Scene/Scene.h"

bool ScenePass::Init(GraphicsDevice& device) {
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

        // [t0, space3] irradiance map (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> irradianceRange;
        irradianceRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 3, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t1, space3] prefiltered map (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> prefilteredRange;
        prefilteredRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 1, 3, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // [t2, space3] BRDF LUT (Descriptor Table SRV)
        std::vector<D3D12_DESCRIPTOR_RANGE1> brdfLutRange;
        brdfLutRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 2, 3, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC));

        // ルートシグニチャ構成
        // [b0] SceneConstants (Root CBV)
        // [b1] TransformConstants (Root CBV)
        // [b2] Material Constants (Root CBV)
        // [b3] Display Constants (Root CBV)
        // [t0-t4] PBR Textures (Descriptor Table SRV)
        // baseColor, metallic-roughness, normal, emissive, occlusion
        // [t0, space1] IES Profile Texture(Descriptor Table SRV)
        // [t0, space2] Light StructuredBuffer (Descriptor Table SRV)
        // [t0, space3] irradiance map (Descriptor Table SRV)
        // [t1, space3] prefiltered map (Descriptor Table SRV)
        // [t2, space3] BRDF LUT (Descriptor Table SRV)
        // [s0] Default Sampler (Static Sampler)
        // [s1] IES Profile Sampler (Static Sampler)
        // [s2] irradiance/LUT Sampler (Static Sampler)
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
            .AddDescriptorTable(irradianceRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(prefilteredRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddDescriptorTable(brdfLutRange, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddStaticSampler(0)
            .AddStaticSampler(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // 垂直角は端で止める
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // 水平角は0-360°でループする
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP)
            .AddStaticSampler(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0,
                D3D12_SHADER_VISIBILITY_PIXEL);

        if (!builder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build root signature.\n");
            return false;
        }

        m_pRootSignature = builder.Get();
    }

    // パイプラインステートの生成
    {
        // シェーダーの読み込み
        std::vector<std::byte> vsData;
        std::vector<std::byte> psData;
        if (!LoadShader(L"shader/SceneVS.cso", vsData) ||
            !LoadShader(L"shader/ScenePS.cso", psData)) {
            OutputDebugStringW(L"Failed to load shaders.\n");
            return false;
        }

        // グラフィックスパイプラインステートの設定
        GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.SetRootSignature(m_pRootSignature.Get())
            .SetVertexShader(vsData.data(), vsData.size())
            .SetPixelShader(psData.data(), psData.size())
            .SetInputLayout(StandardVertex::GetInputLayout())
            .SetBlendState(BlendMode::Opaque)
            .SetDepthMode(DepthMode::Default)
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

    // [t0, space3] irradiance map (共通)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_Irradiance, passBindings.irradianceSRV);

    // [t1, space3] prefiltered env map (共通)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_Prefiltered, passBindings.prefilteredSRV);

    // [t2, space3] BRDF LUT (共通)
    pCmdList->SetGraphicsRootDescriptorTable(
        RootParam::SRV_BrdfLut, passBindings.brdfLutSRV);

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
