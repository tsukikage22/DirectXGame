#include "Engine/Scene/Scene.h"

#include <algorithm>

Scene::Scene()  = default;
Scene::~Scene() = default;

void Scene::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    m_pDevice           = pDevice;
    m_pPoolCBV          = pPoolCBV;
    m_currentFrameIndex = 0;
}

// シーンにモデルを追加する
engine::ModelHandle Scene::RegisterModel(std::unique_ptr<Model> pModel) {
    engine::ModelHandle handle;
    if (pModel) {
        handle = m_modelMap.Insert(std::move(pModel));
    }
    return handle;
}

// アップロードヒープの削除
void Scene::DiscardModelUploads() {
    for (auto& model : m_modelMap) {
        model->DiscardUpload();
    }
}

// シーン内にオブジェクトを作成する
engine::ObjectHandle Scene::SpawnObject(engine::ModelHandle model) {
    // オブジェクトの作成
    auto pObj = std::make_unique<GameObject>(model);

    // transformGPUの初期化
    for (int i = 0; i < config::kFrameCount; i++) {
        pObj->GetTransformGPU(i).Init(
            m_pDevice, m_pPoolCBV, pObj->GetTransform().CalcWorldMatrix());
    }

    // ゲームオブジェクトをシーンに追加
    engine::ObjectHandle handle = m_gameObjectMap.Insert(std::move(pObj));

    return handle;
}

// ゲームオブジェクトの削除
void Scene::DespawnObject(engine::ObjectHandle handle) {
    auto obj = m_gameObjectMap.Erase(handle);
    if (obj.has_value()) {
        m_retireQueue.Retire(std::move(obj.value()), m_currentFrameIndex);

        // Despawn時のm_currentFrameIndexは「前回BeginFrameで設定された値」
        // = このオブジェクトが最後に描画されたフレームスロット。
        // queue[k]のクリアは必ずfence[k]待機の直後に行われるため、
        // GPUがそのスロットの最終描画を終えてから破棄されることが保証される。
        // ※この正しさはBeginFrameでのフェンス待機→クリアの順序に依存する。
        // TODO:将来的にはフェンス値タグ方式への移行が望ましい。
    }
}

void Scene::Term() {
    // モデルの破棄
    m_modelMap.ForEach([](std::unique_ptr<Model>& pModel) { pModel->Term(); });

    // 遅延解放キューのクリア
    m_retireQueue.ClearAll();
}

/// 遅延解放キューのクリア
void Scene::BeginFrame(uint32_t frameIndex) {
    m_currentFrameIndex = frameIndex;
    m_retireQueue.Clear(frameIndex);
}

/// ハンドルに対応するゲームオブジェクトの取得
GameObject* Scene::GetObject(engine::ObjectHandle handle) {
    auto* pObj = m_gameObjectMap.Get(handle);
    return pObj ? pObj->get() : nullptr;
}

/// ハンドルに対応するモデルの取得
Model* Scene::GetModel(engine::ModelHandle handle) {
    auto* pModel = m_modelMap.Get(handle);
    return pModel ? pModel->get() : nullptr;
}
