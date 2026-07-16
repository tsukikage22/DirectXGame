#pragma once

#include <d3d12.h>

#include <memory>
#include <stack>
#include <vector>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GenHandle.h"
#include "Engine/Core/RetireQueue.h"
#include "Engine/Core/SlotMap.h"
#include "Engine/Model/Model.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Shader/TransformGPU.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV);

    void Term();

    /// @brief シーンにモデルを追加する
    engine::ModelHandle RegisterModel(std::unique_ptr<Model> pModel);

    /// @brief モデルのアップロードヒープ破棄
    void DiscardModelUploads();

    /// @brief シーン内にゲームオブジェクトを作成する
    engine::ObjectHandle SpawnObject(engine::ModelHandle model);

    /// @brief ゲームオブジェクトを削除する
    void DespawnObject(engine::ObjectHandle handle);

    /// @brief
    /// フレーム開始時の処理，frameIndexの設定と遅延解放キューのクリア，必ずフェンス待機後に呼び出す
    void BeginFrame(uint32_t frameIndex);

    /// @brief 全ゲームオブジェクトに対してfnを呼び出す
    template <typename Fn>
    void ForEachObject(Fn&& fn) {
        m_gameObjectMap.ForEach(
            // 全てのGameObjectをfnに渡すラムダ式
            [&](std::unique_ptr<GameObject>& pObj) { fn(*pObj); });
    }

    /// @brief ハンドルに対応するゲームオブジェクトの取得
    GameObject* GetObject(engine::ObjectHandle handle);

    /// @brief ハンドルに対応するモデルの取得
    Model* GetModel(engine::ModelHandle handle);

private:
    //==============================================================
    // メンバ変数
    //==============================================================
    // D3D12
    ID3D12Device* m_pDevice    = nullptr;  // デバイス
    DescriptorPool* m_pPoolCBV = nullptr;  // CBV用ディスクリプタプール

    // モデルとオブジェクトのスロットマップ
    SlotMap<std::unique_ptr<GameObject>, engine::GameObjectTag>
        m_gameObjectMap;  // ゲームオブジェクトのスロットマップ
    SlotMap<std::unique_ptr<Model>, engine::ModelTag>
        m_modelMap;  // モデルのスロットマップ

    // 遅延解放キュー
    RetireQueue<std::unique_ptr<GameObject>> m_retireQueue;
    uint32_t m_currentFrameIndex = 0;  // 現在のフレームインデックス
};