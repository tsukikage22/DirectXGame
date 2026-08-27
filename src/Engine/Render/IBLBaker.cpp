#include "Engine/Render/IBLBaker.h"

#include <vector>

#include "Engine/Core/CommandQueue.h"
#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"
#include "Engine/Graphics/ComputePipelineBuilder.h"
#include "Engine/Graphics/RootSignatureBuilder.h"
#include "Engine/Resource/EnvironmentMap.h"
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
    {
        RootSignatureBuilder rsBuilder;
        std::vector<D3D12_DESCRIPTOR_RANGE1> srvRange;
        srvRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));
        std::vector<D3D12_DESCRIPTOR_RANGE1> uavRange;
        uavRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));

        // ルートシグネチャ構成
        // [t0] SRV 入力テクスチャ（equirect / cubemap）
        // [s0] Sampler WRAPはequirectのベイクでのみ有効（irradianceでは無効）
        // [u0] UAV 出力テクスチャ（cubemap / irradiance）
        rsBuilder.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE)
            .AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP)
            .AddDescriptorTable(srvRange, D3D12_SHADER_VISIBILITY_ALL)
            .AddDescriptorTable(uavRange, D3D12_SHADER_VISIBILITY_ALL);
        if (!rsBuilder.Build(m_pDevice->GetDevice())) {
            return false;
        }
        m_pRootSignature = rsBuilder.Get();
    }

    // パイプラインステートの構築
    // Equirect To Cubemap
    {
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
        m_pEquirectPSO = psoBuilder.Get();
    }

    // Env Cubemap Mips
    // RSは同じものが使えるのでPSOだけ作る
    {
        ComputePipelineBuilder psoBuilder;
        std::vector<std::byte> csData;
        if (!LoadShader(L"shader/DownSampleCubemapCS.cso", csData)) {
            OutputDebugStringW(L"Failed to load shaders.\n");
            return false;
        }
        psoBuilder.SetRootSignature(m_pRootSignature.Get())
            .SetComputeShader(csData.data(), csData.size());
        if (!psoBuilder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build compute pipeline state.\n");
            return false;
        }
        m_pEnvmapMipsPSO = psoBuilder.Get();
    }

    // irradiance map
    // RSは同じものが使えるのでPSOだけ作る
    {
        ComputePipelineBuilder psoBuilder;
        std::vector<std::byte> csData;
        if (!LoadShader(L"shader/IrradianceCS.cso", csData)) {
            OutputDebugStringW(L"Failed to load shaders.\n");
            return false;
        }
        psoBuilder.SetRootSignature(m_pRootSignature.Get())
            .SetComputeShader(csData.data(), csData.size());
        if (!psoBuilder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build compute pipeline state.\n");
            return false;
        }
        m_pIrradiancePSO = psoBuilder.Get();
    }

    // prefiltered map
    {
        // ルートシグネチャの構築
        // [t0] SRV 入力テクスチャ（cubemap）
        // [u0] UAV 出力テクスチャ（cubemap）
        // [b0] 定数バッファ roughness
        RootSignatureBuilder rsBuilder;
        std::vector<D3D12_DESCRIPTOR_RANGE1> srvRange;
        srvRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));
        std::vector<D3D12_DESCRIPTOR_RANGE1> uavRange;
        uavRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));
        rsBuilder.AddDescriptorTable(srvRange, D3D12_SHADER_VISIBILITY_ALL)
            .AddDescriptorTable(uavRange, D3D12_SHADER_VISIBILITY_ALL)
            .AddConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_ALL)
            .AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        if (!rsBuilder.Build(m_pDevice->GetDevice())) {
            return false;
        }
        m_pPrefilteredRS = rsBuilder.Get();

        // PSOの構築
        ComputePipelineBuilder psoBuilder;
        std::vector<std::byte> csData;
        if (!LoadShader(L"shader/PrefilteredEnvMapCS.cso", csData)) {
            OutputDebugStringW(L"Failed to load shaders.\n");
            return false;
        }
        psoBuilder.SetRootSignature(m_pPrefilteredRS.Get())
            .SetComputeShader(csData.data(), csData.size());
        if (!psoBuilder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build compute pipeline state.\n");
            return false;
        }
        m_pPrefilteredPSO = psoBuilder.Get();
    }

    // BRDF LUT
    {
        // ルートシグネチャの構築
        // [u0] UAV 出力テクスチャ（2D LUT）
        RootSignatureBuilder rsBuilder;
        std::vector<D3D12_DESCRIPTOR_RANGE1> uavRange;
        uavRange.push_back(
            RootSignatureBuilder::CreateRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE));
        rsBuilder.AddDescriptorTable(uavRange, D3D12_SHADER_VISIBILITY_ALL);
        if (!rsBuilder.Build(m_pDevice->GetDevice())) {
            return false;
        }
        m_pBrdfLutRS = rsBuilder.Get();

        // PSOの構築
        ComputePipelineBuilder psoBuilder;
        std::vector<std::byte> csData;
        if (!LoadShader(L"shader/IntegrateBRDFCS.cso", csData)) {
            OutputDebugStringW(L"Failed to load shaders.\n");
            return false;
        }
        psoBuilder.SetRootSignature(m_pBrdfLutRS.Get())
            .SetComputeShader(csData.data(), csData.size());
        if (!psoBuilder.Build(m_pDevice->GetDevice())) {
            OutputDebugStringW(L"Failed to build compute pipeline state.\n");
            return false;
        }
        m_pBrdfLutPSO = psoBuilder.Get();
    }

    return true;
}

void IBLBaker::Term() {
    if (m_pCommandQueue) {
        m_pCommandQueue->Flush();
    }

    m_pRootSignature    = nullptr;
    m_pPrefilteredRS    = nullptr;
    m_pBrdfLutRS        = nullptr;
    m_pEquirectPSO      = nullptr;
    m_pEnvmapMipsPSO    = nullptr;
    m_pIrradiancePSO    = nullptr;
    m_pPrefilteredPSO   = nullptr;
    m_pBrdfLutPSO       = nullptr;
    m_pDevice           = nullptr;
    m_pCommandAllocator = nullptr;
    m_pCommandList      = nullptr;
    m_pCommandQueue     = nullptr;
}

bool IBLBaker::EquirectToCubemap(EnvironmentMap& envMap) {
    if (!m_pDevice || !m_pCommandQueue) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER before[2] = {};
    // src：PS_SR -> NON_PS_SR
    // dst：PS_SR -> UAV
    before[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[0].Transition.pResource   = envMap.GetEquirectResource();
    before[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[0].Transition.StateAfter =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    before[1].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[1].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[1].Transition.pResource   = envMap.GetCubemapResource();
    before[1].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_pCommandList->ResourceBarrier(2, before);

    // ルートシグネチャとPSOの設定
    m_pCommandList->SetComputeRootSignature(m_pRootSignature.Get());
    m_pCommandList->SetPipelineState(m_pEquirectPSO.Get());

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* ppHeaps[] = { m_pDevice->CbvSrvUavPool()->GetHeap() };
    m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // ルートパラメータの設定
    m_pCommandList->SetComputeRootDescriptorTable(
        RootParam::SRV_Source, envMap.GetEquirectSrvGpuHandle());
    m_pCommandList->SetComputeRootDescriptorTable(
        RootParam::UAV_Dest, envMap.GetCubemapUavGpuHandle());

    // ディスパッチの実行
    uint32_t groupCount = DivRoundUp(envMap.kCubeMapSize, kGroupSize);
    m_pCommandList->Dispatch(groupCount, groupCount, 6);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER after = {};
    // src：NON_PS_SR -> PS_SR
    // dst：直後にmipの生成に進むのでUAVのままにしておく
    after.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after.Transition.pResource   = envMap.GetEquirectResource();
    after.Transition.StateBefore =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    after.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_pCommandList->ResourceBarrier(1, &after);

    // コマンドリストのクローズ
    m_pCommandList->Close();
    // コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->Execute(ppCommandLists, _countof(ppCommandLists));
    // GPUの処理が完了するまで待機
    m_pCommandQueue->Flush();

    return true;
}

bool IBLBaker::GenerateEnvCubemapMips(EnvironmentMap& envMap) {
    if (m_pDevice == nullptr || m_pCommandQueue == nullptr) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // 生成ループ
    // リソースバリアの遷移
    for (uint32_t mip = 1; mip < envMap.kEnvCubeMipLevels; mip++) {
        D3D12_RESOURCE_BARRIER before[6] = {};
        // 6面について，書き込まれたmipをSRVに遷移させていく
        for (uint32_t face = 0; face < 6; face++) {
            // サブリソースのインデックスを計算
            // index = mipSlice + ArraySlice * mipLevel
            const UINT sub = (mip - 1) + face * envMap.kEnvCubeMipLevels;
            // UAVとして書き込まれた前のミップをSRVとして読み込むためにリソースバリアを設定
            before[face].Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            before[face].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            before[face].Transition.Subresource = sub;
            before[face].Transition.pResource   = envMap.GetCubemapResource();
            before[face].Transition.StateBefore =
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            before[face].Transition.StateAfter =
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }
        m_pCommandList->ResourceBarrier(6, before);

        // ルートシグネチャとPSOの設定
        m_pCommandList->SetComputeRootSignature(m_pRootSignature.Get());
        m_pCommandList->SetPipelineState(m_pEnvmapMipsPSO.Get());

        // ディスクリプタヒープの設定
        ID3D12DescriptorHeap* ppHeaps[] = {
            m_pDevice->CbvSrvUavPool()->GetHeap()
        };
        m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        // ルートパラメータの設定
        // srv:mip-1, uav:mip
        m_pCommandList->SetComputeRootDescriptorTable(
            RootParam::SRV_Source, envMap.GetCubemapMipSrvGpuHandle(mip - 1));
        m_pCommandList->SetComputeRootDescriptorTable(
            RootParam::UAV_Dest, envMap.GetCubemapUavGpuHandle(mip));

        // ディスパッチの実行
        uint32_t groupCount =
            DivRoundUp(envMap.kCubeMapSize >> mip, kGroupSize);
        m_pCommandList->Dispatch(groupCount, groupCount, 6);
    }

    // リソースバリアの遷移
    // 最後のmipをNON_PS_SRに遷移させる
    D3D12_RESOURCE_BARRIER after[6] = {};
    for (uint32_t face = 0; face < 6; face++) {
        // サブリソースのインデックスを計算
        // index = mipSlice + ArraySlice * mipLevel
        const UINT sub =
            (envMap.kEnvCubeMipLevels - 1) + face * envMap.kEnvCubeMipLevels;
        after[face].Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        after[face].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        after[face].Transition.Subresource = sub;
        after[face].Transition.pResource   = envMap.GetCubemapResource();
        after[face].Transition.StateBefore =
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        after[face].Transition.StateAfter =
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    m_pCommandList->ResourceBarrier(6, after);
    // 全mipをPS_SRに遷移させる
    D3D12_RESOURCE_BARRIER all = {};
    all.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    all.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    all.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    all.Transition.pResource   = envMap.GetCubemapResource();
    all.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    all.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_pCommandList->ResourceBarrier(1, &all);

    // コマンドリストのクローズ
    m_pCommandList->Close();
    // コマンドリストの実行
    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->Execute(ppCommandLists, _countof(ppCommandLists));
    // GPUの処理が完了するまで待機
    m_pCommandQueue->Flush();

    return true;
}

bool IBLBaker::BakeIrradianceMap(EnvironmentMap& envMap) {
    if (!m_pDevice || !m_pCommandQueue) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER before[2] = {};
    // src：PS_SR -> NON_PS_SR
    // dst：PS_SR -> UAV
    before[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[0].Transition.pResource   = envMap.GetCubemapResource();
    before[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[0].Transition.StateAfter =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    before[1].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[1].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[1].Transition.pResource   = envMap.GetIrradianceResource();
    before[1].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_pCommandList->ResourceBarrier(2, before);

    // ルートシグネチャとPSOの設定
    m_pCommandList->SetComputeRootSignature(m_pRootSignature.Get());
    m_pCommandList->SetPipelineState(m_pIrradiancePSO.Get());

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* ppHeaps[] = { m_pDevice->CbvSrvUavPool()->GetHeap() };
    m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // ルートパラメータの設定
    m_pCommandList->SetComputeRootDescriptorTable(
        RootParam::SRV_Source, envMap.GetCubemapSrvGpuHandle());
    m_pCommandList->SetComputeRootDescriptorTable(
        RootParam::UAV_Dest, envMap.GetIrradianceUavGpuHandle());

    // ディスパッチの実行
    uint32_t groupCount = DivRoundUp(envMap.kIrradianceMapSize, kGroupSize);
    m_pCommandList->Dispatch(groupCount, groupCount, 6);

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER after[2] = {};
    // src：NON_PS_SR -> PS_SR
    // dst：UAV -> PS_SR
    after[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after[0].Transition.pResource   = envMap.GetCubemapResource();
    after[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    after[1].Type                  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[1].Flags                 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after[1].Transition.pResource   = envMap.GetIrradianceResource();
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

bool IBLBaker::BakePrefilteredEnvMap(EnvironmentMap& envMap) {
    if (!m_pDevice || !m_pCommandQueue) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // リソースバリアの遷移
    // src : PS_SR -> NON_PS_SR
    D3D12_RESOURCE_BARRIER before[2] = {};
    before[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[0].Transition.pResource   = envMap.GetCubemapResource();
    before[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[0].Transition.StateAfter =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    // dst : 初期化時点ではPS_SRなので，UAVに遷移させる
    before[1].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before[1].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before[1].Transition.pResource   = envMap.GetPrefilteredResource();
    before[1].Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_pCommandList->ResourceBarrier(2, before);

    // ルートシグネチャとPSOの設定
    m_pCommandList->SetComputeRootSignature(m_pPrefilteredRS.Get());
    m_pCommandList->SetPipelineState(m_pPrefilteredPSO.Get());

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* ppHeaps[] = { m_pDevice->CbvSrvUavPool()->GetHeap() };
    m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // 生成ループ
    for (uint32_t mip = 0; mip < envMap.kPrefilteredMipLevels; mip++) {
        // roughnessの計算
        float roughness = static_cast<float>(mip) /
                          static_cast<float>(envMap.kPrefilteredMipLevels - 1);

        // ルートパラメータの設定
        m_pCommandList->SetComputeRootDescriptorTable(
            RootParam::SRV_Source, envMap.GetCubemapSrvGpuHandle());
        m_pCommandList->SetComputeRootDescriptorTable(
            RootParam::UAV_Dest, envMap.GetPrefilteredUavGpuHandle(mip));
        m_pCommandList->SetComputeRoot32BitConstants(
            RootParam::Constants, 1, &roughness, 0);

        // ディスパッチの実行
        uint32_t groupCount =
            DivRoundUp(envMap.kPrefilteredSize >> mip, kGroupSize);
        m_pCommandList->Dispatch(groupCount, groupCount, 6);
    }

    // リソースバリアの遷移
    D3D12_RESOURCE_BARRIER after[2] = {};
    // src:NON_PS_SR -> PS_SR
    after[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after[0].Transition.pResource   = envMap.GetCubemapResource();
    after[0].Transition.StateBefore =
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    // dst:UAV -> PS_SRに遷移させる
    after[1].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after[1].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after[1].Transition.pResource   = envMap.GetPrefilteredResource();
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

bool IBLBaker::BakeBrdfLut(EnvironmentMap& envMap) {
    if (!m_pDevice || !m_pCommandQueue) {
        return false;
    }

    // コマンドアロケータのリセット
    m_pCommandAllocator->Reset();
    // コマンドリストのリセット
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    // リソースバリアの遷移
    // PS_SR -> UAV
    D3D12_RESOURCE_BARRIER before = {};
    before.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    before.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    before.Transition.pResource   = envMap.GetBrdfLutResource();
    before.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    before.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_pCommandList->ResourceBarrier(1, &before);

    // ルートシグネチャとPSOの設定
    m_pCommandList->SetComputeRootSignature(m_pBrdfLutRS.Get());
    m_pCommandList->SetPipelineState(m_pBrdfLutPSO.Get());

    // ディスクリプタヒープの設定
    ID3D12DescriptorHeap* ppHeaps[] = { m_pDevice->CbvSrvUavPool()->GetHeap() };
    m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // ルートパラメータの設定
    // 書き込み先のUAVだけなので0をいれる
    m_pCommandList->SetComputeRootDescriptorTable(
        0, envMap.GetBrdfLutUavGpuHandle());

    // ディスパッチの実行
    uint32_t groupCount = DivRoundUp(envMap.kBrdfLutSize, kGroupSize);
    m_pCommandList->Dispatch(groupCount, groupCount, 1);

    // リソースバリアの遷移
    // UAV -> PS_SR
    D3D12_RESOURCE_BARRIER after = {};
    after.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    after.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    after.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    after.Transition.pResource   = envMap.GetBrdfLutResource();
    after.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    after.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_pCommandList->ResourceBarrier(1, &after);

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
