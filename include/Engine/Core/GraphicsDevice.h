/// @file GraphicsDevice.h
/// @brief D3D12デバイスの管理

#pragma once

#include <d3d12.h>

#include "Engine/Core/ComPtr.h"

class GraphicsDevice {
public:
    GraphicsDevice()  = default;
    ~GraphicsDevice() = default;

    bool Init();
    void Term();

    ID3D12Device* GetDevice() { return m_pDevice.Get(); }

private:
    engine::ComPtr<ID3D12Device> m_pDevice;  // D3D12デバイス
};