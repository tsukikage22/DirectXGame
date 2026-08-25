#include "Engine/Render/IBLBaker.h"

#include <vector>

#include "Engine/Core/CommandQueue.h"
#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/ComputePipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Resource/ShaderLoader.h"

bool IBLBaker::Init(GraphicsDevice* pDevice) {
    Term();
    if (!pDevice) {
        return false;
    }
    m_pDevice       = pDevice;
    m_pCommandQueue = &pDevice->GetCommandQueue();

    // コマンドアロケータ作成
    CHECK_HR(pDevice->GetDevice(),
        pDevice->GetDevice()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_pCommandAllocator.GetAddressOf())));

    // コマンドリストの生成
    CHECK_HR(m_pDevice->GetDevice(),
        m_pDevice->GetDevice()->CreateCommandList(0,
            D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr,
            IID_PPV_ARGS(m_pCommandList.GetAddressOf())));
    m_pCommandList->Close();

    // ルートシグネチャの構築
    RootSignatureBuilder rsBuilder;
    std::vector<D3D12_DESCRIPTOR_RANGE1> srvRange;
    srvRange.push_back(
        RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
            0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));
    std::vector<D3D12_DESCRIPTOR_RANGE1> uavRange;
    uavRange.push_back(
        RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0,
            0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));

    // ルートシグネチャ構成
    // [t0] SRV: Equirectangular map
    // [s0] Sampler: Linear sampler
    // [u0] UAV: Cubemap
    rsBuilder.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE)
        .AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP)
        .AddDescriptorTable(srvRange, D3D12_SHADER_VISIBILITY_ALL)
        .AddDescriptorTable(uavRange, D3D12_SHADER_VISIBILITY_ALL);
    if (!rsBuilder.Build(m_pDevice->GetDevice())) {
        return false;
    }
    m_pRootSignature = rsBuilder.Get();

    // パイプラインステートの構築
    // Equirect To Cubemap
    ComputePipelineBuilder psoBuilder;
    std::vector<std::byte> csData;
    if (!LoadShader(L"shader/EquirectToCubemapCS.cso", csData)) {
        OutputDebugStringW(L"Failed to load shaders.\n");
        return false;
    }
    psoBuilder.SetRootSignature(m_pRootSignature.Get())
        .SetComputeShader(csData.data(), csData.size());
    if (!psoBuilder.Build(m_pDevice->GetDevice())) {
        OutputDebugStringW(L"Failed to build compute pipeline state.\n");
        return false;
    }
    m_pPSO = psoBuilder.Get();

    // irradiance map（予定）
    // prefiltered map（予定）
    // BRDF LUT（予定）

    return true;
}

void IBLBaker::Term() {
    if (m_pCommandQueue) {
        m_pCommandQueue->Flush();
    }

    m_pRootSignature    = nullptr;
    m_pPSO              = nullptr;
    m_pDevice           = nullptr;
    m_pCommandAllocator = nullptr;
    m_pCommandList      = nullptr;
    m_pCommandQueue     = nullptr;
}

bool IBLBaker::EquirectToCubemap(ID3D12Resource* srcRes, ID3D12Resource* dstRes,
    D3D12_GPU_DESCRIPTOR_HANDLE srcEquirectSRV,
    D3D12_GPU_DESCRIPTOR_HANDLE dstCubemapUAV, uint32_t size) {
    if (!m_pDevice || !m_pCommandQueue) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER before = {};
    // src：PS_SR -> NON_PS_SR
    // dst：UAVで作成するので不要
    before.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before.Transition.pResource   = srcRes;
    before.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before.Transition.StateAfter =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    m_pCommandList->ResourceBarrier(1, &before);

    // ルートシグネチャとPSOの設定
    m_pCommandList->SetComputeRootSignature(m_pRootSignature.Get());
    m_pCommandList->SetPipelineState(m_pPSO.Get());

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* ppHeaps[] = { m_pDevice->CbvSrvUavPool()->GetHeap() };
    m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // ルートパラメータの設定
    m_pCommandList->SetComputeRootDescriptorTable(
        CubemapRootParam::EquirectSRV, srcEquirectSRV);
    m_pCommandList->SetComputeRootDescriptorTable(
        CubemapRootParam::CubemapUAV, dstCubemapUAV);

    // ディスパッチの実行
    uint32_t groupCount = DivRoundUp(size, kGroupSize);
    m_pCommandList->Dispatch(groupCount, groupCount, 6);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER after[2] = {};
    // src：NON_PS_SR -> PS_SR
    // dst：UAV -> PS_SR
    after[0].Type                 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[0].Flags                = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[0].Transition.pResource = srcRes;
    after[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    after[1].Type                  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[1].Flags                 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[1].Transition.pResource  = dstRes;
    after[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    after[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_pCommandList->ResourceBarrier(2, after);

    // コマンドリストのクローズ
    m_pCommandList->Close();
    // コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->Execute(ppCommandLists, _countof(ppCommandLists));
    // GPUの処理が完了するまで待機
    m_pCommandQueue->Flush();

    return true;
}

uint32_t IBLBaker::DivRoundUp(uint32_t value, uint32_t divisor) {
    return (value + divisor - 1) / divisor;
}
