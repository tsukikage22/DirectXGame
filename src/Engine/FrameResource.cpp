#include "FrameResource.h"

FrameResource::FrameResource()
    : m_pCmdAllocator(nullptr),
      m_transforms(),
      m_sceneConstants(),
      m_fenceValue(0) {}

FrameResource::~FrameResource() { Term(); }

bool FrameResource::Init(
    ID3D12Device* pDevice, DescriptorPool* pPoolCBV, size_t numObjects) {
    // 引数チェック
    if (!pDevice || !pPoolCBV) {
        return false;
    }

    // コマンドアロケータ作成
    auto hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_pCmdAllocator.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    // TransformGPU初期化
    m_transforms.resize(numObjects);
    for (size_t i = 0; i < numObjects; ++i) {
        if (!m_transforms[i].Init(pDevice, pPoolCBV)) {
            return false;
        }
    }

    // SceneConstants初期化
    if (!m_sceneConstants.Init(pDevice, pPoolCBV)) {
        return false;
    }

    return true;
}

void FrameResource::Term() {
    for (auto& transform : m_transforms) {
        transform.Term();
    }
    m_transforms.clear();
    m_sceneConstants.Term();
    m_pCmdAllocator.Reset();
}

void FrameResource::BeginFrame(ID3D12GraphicsCommandList* pCmdList) {
    // コマンドアロケータのリセット
    m_pCmdAllocator->Reset();

    // コマンドリストのリセット
    pCmdList->Reset(m_pCmdAllocator.Get(), nullptr);
}

void FrameResource::EndFrame(UINT64 fenceValue) { m_fenceValue = fenceValue; }
