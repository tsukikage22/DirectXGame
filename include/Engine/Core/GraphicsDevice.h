/// @file GraphicsDevice.h
/// @brief D3D12デバイスの管理

#pragma once

#include <d3d12.h>

#include <memory>

#include "Engine/Core/ComPtr.h"
#include "Engine/Core/CommandQueue.h"
#include "Engine/Core/DescriptorPool.h"

class GraphicsDevice {
public:
    GraphicsDevice()  = default;
    ~GraphicsDevice() = default;

    bool Init();
    void Term();

    /// @brief GPUの処理が完了するまで待機
    void WaitForGPU() { m_CommandQueue.Flush(); }

    ID3D12Device* GetDevice() { return m_pDevice.Get(); }
    CommandQueue& GetCommandQueue() { return m_CommandQueue; }
    DescriptorPool* CbvSrvUavPool() { return m_pPoolCBV_SRV_UAV.get(); }
    DescriptorPool* RtvPool() { return m_pPoolRTV.get(); }
    DescriptorPool* DsvPool() { return m_pPoolDSV.get(); }
    DescriptorPool* SmpPool() { return m_pPoolSMP.get(); }

private:
    engine::ComPtr<ID3D12Device> m_pDevice;  // D3D12デバイス
    CommandQueue m_CommandQueue;             // コマンドキュー

    // ディスクリプタプール
    std::unique_ptr<DescriptorPool> m_pPoolCBV_SRV_UAV;  // CBV/SRV/UAV用
    std::unique_ptr<DescriptorPool> m_pPoolRTV;          // RTV用
    std::unique_ptr<DescriptorPool> m_pPoolDSV;          // DSV用
    std::unique_ptr<DescriptorPool> m_pPoolSMP;          // サンプラ用
};