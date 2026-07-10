#pragma once

#include <memory>
#include <stack>
#include <vector>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GenHandle.h"
#include "Engine/Core/SlotMap.h"
#include "Engine/Model/Model.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Shader/TransformGPU.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Term();

    /// @brief シーンにモデルを追加する
    engine::ModelHandle RegisterModel(std::unique_ptr<Model> pModel);

    /// @brief モデルのアップロードヒープ破棄
    void DiscardModelUploads();

    /// @brief シーン内にゲームオブジェクトを作成する
    engine::ObjectHandle CreateGameObject(engine::ModelHandle model,
        ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    /// @brief ゲームオブジェクトを削除する
    void RemoveGameObject(engine::ObjectHandle handle);

    /// @brief 全ゲームオブジェクトに対してfnを呼び出す
    template <typename Fn>
    void ForEachObject(Fn&& fn) {
        m_gameObjectMap.ForEach(
            // 全てのGameObjectをfnに渡すラムダ式
            [&](std::unique_ptr<GameObject>& pObj) { fn(*pObj); });
    }

    /// @brief ハンドルに対応するゲームオブジェクトの取得
    GameObject* GetObject(engine::ObjectHandle handle) {
        auto* pObj = m_gameObjectMap.Get(handle);
        return pObj ? pObj->get() : nullptr;
    }

    /// @brief ハンドルに対応するモデルの取得
    Model* GetModel(engine::ModelHandle handle) {
        auto* pModel = m_modelMap.Get(handle);
        return pModel ? pModel->get() : nullptr;
    }

private:
    //==============================================================
    // メンバ変数
    //==============================================================
    SlotMap<std::unique_ptr<GameObject>, engine::GameObjectTag>
        m_gameObjectMap;  // ゲームオブジェクトのスロットマップ
    SlotMap<std::unique_ptr<Model>, engine::ModelTag>
        m_modelMap;  // モデルのスロットマップ
};