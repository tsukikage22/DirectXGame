#include "Engine/Scene.h"

#include <algorithm>

Scene::Scene()  = default;
Scene::~Scene() = default;

Model* Scene::AddModel(std::unique_ptr<Model> pModel) {
    if (!pModel) {
        return nullptr;
    }

    m_models.push_back(std::move(pModel));
    return m_models.back().get();
}

void Scene::DiscardModelUploads() {
    for (auto& model : m_models) {
        model->DiscardUpload();
    }
}

uint32_t Scene::CreateGameObject(Model* pModel) {
    auto pObj = std::make_unique<GameObject>(pModel);
    m_gameObjects.push_back(std::move(pObj));

    // objectIndexの割り当て
    uint32_t objectIndex;
    if (m_freeSlots.empty()) {
        // フリーリストが空の場合は新しいスロットを追加
        objectIndex = m_nextObjectIndex++;
        m_gameObjects.back()->SetIndex(objectIndex);
    } else {
        // フリーリストからスロットを割り当て
        objectIndex = m_freeSlots.top();
        m_freeSlots.pop();
        m_gameObjects.back()->SetIndex(objectIndex);
    }

    return objectIndex;
}

void Scene::RemoveGameObject(uint32_t index) {
    // indexに対応するゲームオブジェクトを検索
    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [index](const std::unique_ptr<GameObject>& obj) {
            return obj->GetIndex() == index;
        });

    // オブジェクトの削除とフリーリストへの追加
    if (it != m_gameObjects.end()) {
        m_gameObjects.erase(it);
        m_freeSlots.push(index);
    }
}

void Scene::Term() {
    // モデルの破棄
    for (auto& model : m_models) {
        model->Term();
    }
    m_models.clear();

    // ゲームオブジェクトの破棄
    m_gameObjects.clear();
}