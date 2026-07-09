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
    m_gameObjectMap.Erase(handle);
}

void Scene::Term() {
    // モデルの破棄
    m_modelMap.ForEach([](std::unique_ptr<Model>& pModel) { pModel->Term(); });
}