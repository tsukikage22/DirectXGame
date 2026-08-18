#include "Engine/Core/GraphicsDevice.h"

#include "Engine/Core/EngineConfig.h"

bool GraphicsDevice::Init() {
    // デバイスの生成
    auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(m_pDevice.GetAddressOf()));
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to create D3D12 Device.\n");
        return false;
    }

    // コマンドキュー・フェンスの生成
    if (!m_CommandQueue.Init(m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)) {
        return false;
    }

    // ディスクリプタプールの生成
    // CBV/SRV/UAV
    m_pPoolCBV_SRV_UAV = DescriptorPool::Create(m_pDevice.Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, config::kCbvSrvUavCapacity);
    if (!m_pPoolCBV_SRV_UAV) {
        return false;
    }

    // RTV
    m_pPoolRTV =
        DescriptorPool::Create(m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE, config::kRtvCapacity);
    if (!m_pPoolRTV) {
        return false;
    }

    // DSV
    m_pPoolDSV =
        DescriptorPool::Create(m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE, config::kDsvCapacity);
    if (!m_pPoolDSV) {
        return false;
    }

    // SMP
    m_pPoolSMP = DescriptorPool::Create(m_pDevice.Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, config::kSamplerCapacity);
    if (!m_pPoolSMP) {
        return false;
    }

    return true;
}

void GraphicsDevice::Term() {
    // GPUの処理が完了するまで待機
    m_CommandQueue.Flush();

    // ディスクリプタプールの破棄（デバイスより先に）
    m_pPoolCBV_SRV_UAV.reset();
    m_pPoolRTV.reset();
    m_pPoolDSV.reset();
    m_pPoolSMP.reset();

    // デバイスより先にコマンドキューを破棄する必要がある
    m_CommandQueue.Term();

    // デバイスの破棄
    m_pDevice.Reset();
}