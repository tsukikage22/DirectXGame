#include "Engine/Shader/LightBuffer.h"

#include <cassert>

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/EngineConfig.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Shader/ShaderConstants.h"

LightBuffer::LightBuffer() : m_pPool(nullptr), m_pMappedData(nullptr) {}

LightBuffer::~LightBuffer() { Term(); }

bool LightBuffer::Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV) {
    // 引数チェック
    if (pDevice == nullptr || pPoolSRV == nullptr) {
        return false;
    }

    m_pPool      = pPoolSRV;
    m_allocation = pPoolSRV->Allocate();
    if (!m_allocation.IsValid()) {
        return false;
    }

    // バッファの作成
    if (!m_buffer.CreateDynamic(
            pDevice, sizeof(shader::LightConstants) * config::kMaxLights)) {
        return false;
    }

    // メモリマッピング
    m_pMappedData = m_buffer.GetMappedPtr();
    if (m_pMappedData == nullptr) {
        m_buffer.Term();
        return false;
    }

    // SRVの作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
    srv.Format                          = DXGI_FORMAT_UNKNOWN;
    srv.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
    srv.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Buffer.FirstElement        = 0;
    srv.Buffer.NumElements         = config::kMaxLights;
    srv.Buffer.StructureByteStride = sizeof(shader::LightConstants);
    srv.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;

    pDevice->CreateShaderResourceView(
        m_buffer.GetResource(), &srv, m_allocation.GetCPUHandle());

    return true;
}

void LightBuffer::Term() {
    m_buffer.Term();
    m_pPool       = nullptr;
    m_pMappedData = nullptr;
    m_allocation  = DescriptorAllocation{};
}

uint32_t LightBuffer::Update(
    const shader::LightConstants* pLights, uint32_t count) {
    // 引数チェック
    if (pLights == nullptr || m_pMappedData == nullptr) {
        return 0;
    }

    // ライトの最大数を超えた場合は切り捨てる
    if (count > config::kMaxLights) {
        assert(false && "light count exceeds maximum limit");
        count = config::kMaxLights;
    }

    // データのコピー
    memcpy(m_pMappedData, pLights, sizeof(shader::LightConstants) * count);

    return count;
}

D3D12_GPU_DESCRIPTOR_HANDLE LightBuffer::GetGPUHandle() const {
    return m_allocation.GetGPUHandle();
}