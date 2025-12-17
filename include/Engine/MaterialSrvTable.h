#pragma once

#include <d3d12.h>

#include <cassert>
#include <optional>

class DescriptorPool;
class TextureManager;
class MaterialGPU;

/// @brief マテリアル用のSRVテーブルの管理
class MaterialSrvTable {
public:
    MaterialSrvTable() = default;

    /// @brief SRVプール上の連続領域の確保
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
        const MaterialGPU* pMaterialGPU, TextureManager* pTextureManager);

    /// @brief 確保した領域の解放
    void Term();

    /// @brief 先頭SRVのGPUハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseGPUHandle() const;

private:
    DescriptorPool* m_pPoolSRV = nullptr;

    std::optional<uint32_t> m_SRVbase = std::nullopt;
};