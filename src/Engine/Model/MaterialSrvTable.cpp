#include "Engine/Model/MaterialSrvTable.h"

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Model/MaterialGPU.h"
#include "Engine/Resource/TextureManager.h"

bool MaterialSrvTable::Init(ID3D12Device* pDevice, DescriptorPool* pPoolSRV,
    const MaterialGPU* pMaterialGPU, TextureManager* pTextureManager) {
    // 引数チェック
    if (pDevice == nullptr || pPoolSRV == nullptr) {
        return false;
    }

    m_pPoolSRV = pPoolSRV;

    // 連続領域の確保
    m_SRVbase = m_pPoolSRV->AllocateRange(
        static_cast<uint32_t>(TextureUsage::Count));  // マテリアル用に5つ確保
    if (!m_SRVbase.IsValid()) {
        assert(false && "Failed to allocate SRV range for MaterialSrvTable");
        return false;
    }

    // 各スロットを埋める
    for (int i = 0; i < static_cast<int>(TextureUsage::Count); i++) {
        // このスロットに対応するテクスチャを取得
        std::optional<uint32_t> textureIndex = std::nullopt;
        switch (static_cast<TextureUsage>(i)) {
            case TextureUsage::BaseColor:
                textureIndex =
                    pMaterialGPU->GetTextureHandle(TextureUsage::BaseColor);
                break;
            case TextureUsage::MetallicRoughness:
                textureIndex = pMaterialGPU->GetTextureHandle(
                    TextureUsage::MetallicRoughness);
                break;
            case TextureUsage::Normal:
                textureIndex =
                    pMaterialGPU->GetTextureHandle(TextureUsage::Normal);
                break;
            case TextureUsage::Emissive:
                textureIndex =
                    pMaterialGPU->GetTextureHandle(TextureUsage::Emissive);
                break;
            case TextureUsage::Occlusion:
                textureIndex =
                    pMaterialGPU->GetTextureHandle(TextureUsage::Occlusion);
                break;
        }

        // AssetSRVのCPUハンドルを取得
        D3D12_CPU_DESCRIPTOR_HANDLE srcCPUHandle{};
        if (textureIndex.has_value()) {
            srcCPUHandle = pTextureManager->GetSrvCpuHandle(
                TextureHandle{ textureIndex.value() });
        } else {
            // テクスチャが無ければデフォルトを使う
            switch (static_cast<TextureUsage>(i)) {
                case TextureUsage::Normal:
                    srcCPUHandle =
                        pTextureManager->GetNormalFlat()->GetDefaultSrvCpu();
                    break;
                default:
                    srcCPUHandle =
                        pTextureManager->GetWhiteDefault()->GetDefaultSrvCpu();
                    break;
            }
        }

        if (!srcCPUHandle.ptr) {
            assert(false &&
                   "Failed to get source CPU handle for MaterialSrvTable");
            return false;
        }

        // MaterialSRVにコピー
        D3D12_CPU_DESCRIPTOR_HANDLE destCPUHandle = m_SRVbase.GetCPUHandle(i);

        pDevice->CopyDescriptorsSimple(1, destCPUHandle, srcCPUHandle,
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    return true;
}

void MaterialSrvTable::Term() {
    // 連続領域のプールからの解放はデストラクタによって行われるのでやらない
    m_pPoolSRV = nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE MaterialSrvTable::GetBaseGPUHandle() const {
    if (m_pPoolSRV == nullptr || !m_SRVbase.IsValid()) {
        return {};
    }

    return m_SRVbase.GetGPUHandle();
}