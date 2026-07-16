#pragma once

#include <memory>

#include "Engine/Core/GenHandle.h"
#include "Engine/Input/InputSystem.h"

// 前方宣言
class Engine;
class CameraController;
class GameObject;

class Game {
public:
    Game();
    ~Game();

    // 初期化
    void Init(Engine* pEngine);

    // 終了処理
    void Term();

    // 更新
    void Tick(float deltaTime);

private:
    Engine* m_pEngine;
    InputSystem* m_pInputSystem;
    std::unique_ptr<CameraController> m_pCameraController;

    engine::ModelHandle m_earthModel;  // シーン内の球体モデルのハンドル
    engine::ModelHandle m_moonModel;

    engine::ObjectHandle
        m_earthObject;  // シーン内のゲームオブジェクトのハンドル
    engine::ObjectHandle
        m_moonObject;  // シーン内のゲームオブジェクトのハンドル
};