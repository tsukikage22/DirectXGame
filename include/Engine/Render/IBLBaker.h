
/// @file IBLBaker.h
/// @brief IBL（Image-Based Lighting）用のベイク処理を行う

#pragma once

#include <d3d12.h>

#include <cstdint>

#include "Engine/Core/ComPtr.h"

class GraphicsDevice;
class CommandQueue;
class EnvironmentMap;

class IBLBaker {
public:
    IBLBaker()  = default;
    ~IBLBaker() = default;

    bool Init(GraphicsDevice* pDevice);

    void Term();

    /// @brief キューブマップの作成
    /// @param envMap 環境マップ
    /// @return キューブマップの作成に成功したかどうか
    [[nodiscard]] bool EquirectToCubemap(EnvironmentMap& envMap);

private:
    static uint32_t DivRoundUp(uint32_t value, uint32_t divisor);

    enum CubemapRootParam {
        EquirectSRV = 0,  // [t0] SRV: Equirectangular map
        CubemapUAV,       // [u0] UAV: Cubemap
        Count
    };

    constexpr static uint32_t kGroupSize = 8;

    GraphicsDevice* m_pDevice     = nullptr;
    CommandQueue* m_pCommandQueue = nullptr;

    engine::ComPtr<ID3D12CommandAllocator> m_pCommandAllocator = nullptr;
    engine::ComPtr<ID3D12GraphicsCommandList> m_pCommandList   = nullptr;
    engine::ComPtr<ID3D12RootSignature> m_pRootSignature       = nullptr;
    engine::ComPtr<ID3D12PipelineState> m_pPSO                 = nullptr;
};