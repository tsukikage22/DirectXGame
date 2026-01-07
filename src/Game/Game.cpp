#include "Game/Game.h"

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
}

void Game::Tick() {
    // カメラ操作の更新
    if (m_pCameraController) {
        m_pCameraController->Update();
    }
}