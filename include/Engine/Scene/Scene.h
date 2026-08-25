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
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Light.h"
#include "Engine/Shader/TransformGPU.h"

// 前方宣言
class GraphicsDevice;

class Scene {
public:
    Scene();
    ~Scene();

    void Init(GraphicsDevice& graphicsDevice);

    void Term();

    /// @brief シーンにモデルを追加する
    engine::ModelHandle RegisterModel(std::unique_ptr<Model> pModel);

    /// @brief モデルのアップロードヒープ破棄
    void DiscardModelUploads();

    /// @brief シーン内にゲームオブジェクトを作成する
    engine::ObjectHandle SpawnObject(engine::ModelHandle model);

    /// @brief ゲームオブジェクトを削除する
    void DespawnObject(engine::ObjectHandle handle);

    /// @brief ライトの作成
    engine::LightHandle SpawnDirectionalLight(const DirectionalLightDesc& desc);
    engine::LightHandle SpawnPointLight(const PointLightDesc& desc);
    engine::LightHandle SpawnSpotLight(const SpotLightDesc& desc);
    engine::LightHandle SpawnPhotometricLight(const PhotometricLightDesc& desc);

    /// @brief ライトの削除
    void DespawnLight(engine::LightHandle handle);

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

    /// @brief 全ライトに対してfnを呼び出す
    template <typename Fn>
    void ForEachLight(Fn&& fn) {
        m_lightMap.ForEach(
            // 全てのLightをfnに渡すラムダ式
            [&](std::unique_ptr<Light>& pLight) { fn(*pLight); });
    }

    /// @brief ハンドルに対応するゲームオブジェクトの取得
    GameObject* GetObject(engine::ObjectHandle handle);

    /// @brief ハンドルに対応するモデルの取得
    Model* GetModel(engine::ModelHandle handle);

    /// @brief ハンドルに対応するライトの取得
    Light* GetLight(engine::LightHandle handle);

    /// @brief シーンのカメラを取得する
    Camera& GetCamera() { return m_camera; }

    /// @brief 環境マップの輝度スケール係数を設定する
    void SetEnvIntensity(float intensity) { m_envIntensity = intensity; }

    /// @brief 環境マップの輝度スケール係数を取得する
    float GetEnvIntensity() const { return m_envIntensity; }

private:
    //==============================================================
    // メンバ変数
    //==============================================================
    // D3D12
    ID3D12Device* m_pDevice    = nullptr;  // デバイス
    DescriptorPool* m_pPoolCBV = nullptr;  // CBV用ディスクリプタプール

    // スロットマップ
    SlotMap<std::unique_ptr<GameObject>, engine::GameObjectTag>
        m_gameObjectMap;  // ゲームオブジェクトのスロットマップ
    SlotMap<std::unique_ptr<Model>, engine::ModelTag>
        m_modelMap;  // モデルのスロットマップ
    SlotMap<std::unique_ptr<Light>, engine::LightTag>
        m_lightMap;  // ライトのスロットマップ

    // カメラ
    Camera m_camera;  // シーンのカメラ

    float m_envIntensity = 1.0f;  // 環境マップの輝度スケール係数

    // 遅延解放キュー
    RetireQueue<std::unique_ptr<GameObject>> m_retireQueue;
    uint32_t m_currentFrameIndex = 0;  // 現在のフレームインデックス
};