/// @file LightBuffer.h
/// @brief ライト定数バッファの構造体

#pragma once

#include <d3d12.h>

#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Shader/ShaderConstants.h"

class DescriptorPool;

class LightBuffer
{
public:
    LightBuffer();
    ~LightBuffer();

    /// @brief StructuredBufferの初期化
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV);

    void Term();

    /// @brief ライトバッファの更新
    /// @param pLights バッファにコピーする配列
    /// @param count コピーする個数
    /// @return 実際にコピーされた個数
    uint32_t Update(const shader::LightConstants* pLights, uint32_t count);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

private:
    GPUBuffer m_buffer;                // ライトバッファ
    DescriptorPool* m_pPool;           // ディスクリプタプール
    DescriptorAllocation m_allocation; // ディスクリプタの割り当て
    void* m_pMappedData;               // マップ済みデータ

    // コピー禁止
    LightBuffer(const LightBuffer&)            = delete;
    LightBuffer& operator=(const LightBuffer&) = delete;
};
