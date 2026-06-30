#pragma once

#include <memory>
#include <stack>
#include <vector>

#include "Engine/Model/Model.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Shader/TransformGPU.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Term();

    /// @brief シーンにモデルを追加する
    Model* AddModel(std::unique_ptr<Model> pModel);

    /// @brief モデルのアップロードヒープ破棄
    void DiscardModelUploads();

    /// @brief シーン内にゲームオブジェクトを作成する
    uint32_t CreateGameObject(
        Model* pModel, ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    /// @brief ゲームオブジェクトを削除する
    void RemoveGameObject(uint32_t index);

    /// @brief インデックスに対応するゲームオブジェクトを取得
    GameObject* GetGameObject(uint32_t index);

    /// @brief ゲームオブジェクト配列の取得
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const {
        return m_gameObjects;
    }

private:
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<std::unique_ptr<Model>> m_models;

    uint32_t m_nextObjectIndex = 0;    // 次に割り当てるオブジェクトインデックス
    std::stack<uint32_t> m_freeSlots;  // transformGPUのフリーリスト
};