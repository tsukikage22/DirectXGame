#include "Engine/Core/FrameResource.h"

#include "Engine/Core/DxDebug.h"

FrameResource::FrameResource()
    : m_pCmdAllocator(nullptr), m_sceneConstants(), m_fenceValue(0) {}

FrameResource::~FrameResource() { Term(); }

bool FrameResource::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // コマンドアロケータ作成
    CHECK_HR(
        pDevice, pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                     IID_PPV_ARGS(m_pCmdAllocator.GetAddressOf())));

    // SceneConstants初期化
    if (!m_sceneConstants.Init(pDevice, pPoolCBV)) {
        return false;
    }

    // LightBuffer初期化
    if (!m_lightBuffer.Init(pDevice, pPoolCBV)) {
        return false;
    }

    return true;
}

void FrameResource::Term() {
    // 初期化前に呼ばれても安全
    if (!m_pCmdAllocator) {
        return;
    }

    // リソースの解放
    m_sceneConstants.Term();
    m_lightBuffer.Term();
    m_pCmdAllocator.Reset();
}

void FrameResource::BeginFrame(ID3D12GraphicsCommandList* pCmdList) {
    // コマンドアロケータのリセット
    m_pCmdAllocator->Reset();

    // コマンドリストのリセット
    pCmdList->Reset(m_pCmdAllocator.Get(), nullptr);
    m_isActive = true;
}

void FrameResource::EndFrame(UINT64 fenceValue) {
    m_fenceValue = fenceValue;
    m_isActive   = false;
}
