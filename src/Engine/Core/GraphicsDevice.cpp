#include "Engine/Core/GraphicsDevice.h"

bool GraphicsDevice::Init() {
    // デバイスの生成
    auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(m_pDevice.GetAddressOf()));
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to create D3D12 Device.\n");
        return false;
    }
    return true;
}

void GraphicsDevice::Term() { m_pDevice.Reset(); }