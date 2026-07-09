#include "Game/Game.h"

#include <DirectXMath.h>

#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "Game/CameraController.h"

Game::Game()
    : m_pEngine(nullptr),
      m_pCameraController(std::make_unique<CameraController>()) {}

Game::~Game() {}

void Game::Init(Engine* pEngine) {
    m_pEngine = pEngine;

    m_pCameraController = std::make_unique<CameraController>();

    // カメラコントローラの初期化
    m_pCameraController->Init(
        &m_pEngine->GetCamera(), &m_pEngine->GetInputSystem());
}

void Game::Tick(float deltaTime) {
    // カメラ操作の更新
    if (m_pCameraController) {
        m_pCameraController->Update(deltaTime);
    }

    // ゲームオブジェクトの更新
    Scene& scene = m_pEngine->GetScene();
    scene.ForEachObject([deltaTime](GameObject& obj) {
        DirectX::XMFLOAT3 rot = obj.GetTransform().GetRotation();
        rot.y += 2.0f * deltaTime;  // 毎フレーム少しずつ回転
        obj.GetTransform().SetRotation(rot);
    });
}