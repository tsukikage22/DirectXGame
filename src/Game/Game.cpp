#include "Game/Game.h"

#include <DirectXMath.h>

#include "Engine/Engine.h"
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

    // ゲームオブジェクトの取得
    auto& scene = m_pEngine->GetScene();
    if (!scene.GetGameObjects().empty()) {
        m_pObject1 = scene.GetGameObject(0);
        m_pObject2 = scene.GetGameObject(1);
    }
}

void Game::Tick(float deltaTime) {
    // カメラ操作の更新
    if (m_pCameraController) {
        m_pCameraController->Update(deltaTime);
    }

    // ゲームオブジェクトの更新
    if (m_pObject1) {
        DirectX::XMFLOAT3 rot = m_pObject1->GetTransform().GetRotation();
        rot.y += 2.0f * deltaTime;  // 毎フレーム少しずつ回転
        m_pObject1->GetTransform().SetRotation(rot);
    }
    if (m_pObject2) {
        DirectX::XMFLOAT3 rot = m_pObject2->GetTransform().GetRotation();
        rot.y -= 2.0f * deltaTime;  // 毎フレーム少しずつ回転
        m_pObject2->GetTransform().SetRotation(rot);
    }
}