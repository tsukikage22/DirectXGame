#include "Game/Game.h"

#include <DirectXMath.h>

#include "Engine/Engine.h"
#include "Engine/Resource/AssetPath.h"
#include "Engine/Resource/ModelLoadScope.h"
#include "Engine/Scene/Scene.h"
#include "Game/CameraController.h"

Game::Game()
    : m_pEngine(nullptr),
      m_pCameraController(std::make_unique<CameraController>()) {}

Game::~Game() {}

void Game::Init(Engine* pEngine) {
    m_pEngine      = pEngine;
    m_pInputSystem = &m_pEngine->GetInputSystem();

    m_pCameraController = std::make_unique<CameraController>();

    // カメラコントローラの初期化
    m_pCameraController->Init(
        &m_pEngine->GetCamera(), &m_pEngine->GetInputSystem());

    // モデルのロード
    auto loader = m_pEngine->CreateModelLoadScope();
    std::filesystem::path path;
    AssetPath().GetAssetPath(L"model/TextureSphere.glb", path);
    m_earthModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/MoonSphere.glb", path);
    m_moonModel = loader.LoadModel(path);
}

void Game::Tick(float deltaTime) {
    // カメラ操作の更新
    if (m_pCameraController) {
        m_pCameraController->Update(deltaTime);
    }

    // ゲームオブジェクトの生成削除
    if (m_pInputSystem->WasKeyPressed('1')) {
        if (!m_earthObject.IsValid()) {
            m_earthObject = m_pEngine->GetScene().SpawnObject(m_earthModel);
        } else {
            m_pEngine->GetScene().DespawnObject(m_earthObject);
            m_earthObject = {};
        }
    }

    if (m_pInputSystem->WasKeyPressed('2')) {
        if (!m_moonObject.IsValid()) {
            m_moonObject = m_pEngine->GetScene().SpawnObject(m_moonModel);
        } else {
            m_pEngine->GetScene().DespawnObject(m_moonObject);
            m_moonObject = {};
        }
    }
}