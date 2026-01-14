#include "Engine/FrameResource.h"

#include "Engine/DxDebug.h"

FrameResource::FrameResource()
    : m_pCmdAllocator(nullptr),
      m_pTransforms(),
      m_sceneConstants(),
      m_fenceValue(0) {}

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

    // LightingConstants初期化
    if (!m_lightingConstants.Init(pDevice, pPoolCBV)) {
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
    m_pTransforms.clear();
    m_sceneConstants.Term();
    m_lightingConstants.Term();
    m_pCmdAllocator.Reset();
}

bool FrameResource::AddTransform(
    ID3D12Device* pDevice, DescriptorPool* pPoolCBV, size_t count) {
    // メモリ確保
    size_t curSize = m_pTransforms.size();
    m_pTransforms.reserve(m_pTransforms.size() + count);

    // Transformの初期化と追加
    for (size_t i = 0; i < count; i++) {
        auto transform = std::make_unique<TransformGPU>();
        if (!transform->Init(pDevice, pPoolCBV)) {
            // 追加分の削除
            m_pTransforms.resize(curSize);
            return false;
        }
        m_pTransforms.push_back(std::move(transform));
    }
    return true;
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
