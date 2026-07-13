#include "Engine/Scene/Scene.h"

#include <algorithm>

Scene::Scene()  = default;
Scene::~Scene() = default;

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
engine::ObjectHandle Scene::CreateGameObject(engine::ModelHandle model,
    ID3D12Device* pDevice, DescriptorPool* pPoolCBV) {
    // オブジェクトの作成
    auto pObj = std::make_unique<GameObject>(model);

    // transformGPUの初期化
    for (int i = 0; i < config::kFrameCount; i++) {
        pObj->GetTransformGPU(i).Init(
            pDevice, pPoolCBV, pObj->GetTransform().CalcWorldMatrix());
    }

    // ゲームオブジェクトをシーンに追加
    engine::ObjectHandle handle = m_gameObjectMap.Insert(std::move(pObj));

    return handle;
}

// ゲームオブジェクトの削除
void Scene::RemoveGameObject(engine::ObjectHandle handle) {
    auto obj = m_gameObjectMap.Erase(handle);
    if (obj.has_value()) {
        m_retireQueue.Retire(std::move(obj.value()), m_currentFrameIndex);
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
