#include "Game/Game.h"

#include <DirectXMath.h>

#include "Engine/Engine.h"
#include "Engine/Resource/AssetLoadScope.h"
#include "Engine/Resource/AssetPath.h"
#include "Engine/Scene/GameObject.h"
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
    auto loader = m_pEngine->CreateAssetLoadScope();
    std::filesystem::path path;
    AssetPath().GetAssetPath(L"model/TextureSphere.glb", path);
    m_earthModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/lowpoly_apple.glb", path);
    m_appleModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/Katana.glb", path);
    m_katanaModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/Plane.glb", path);
    m_planeModel = loader.LoadModel(path);

    // ライトの作成
    engine::LightHandle lightHandle;

    // Directional
    /*
    {
        lightHandle  = m_pEngine->GetScene().SpawnLight(LightType::Directional);
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetRotation(90.0f, 0.0f, 0.0f);
        pLight->SetIlluminance(1.5f);
        pLight->SetColor({ 1.0f, 1.0f, 1.0f });
    }
    */

    // Point
    /*
    {
        lightHandle  = m_pEngine->GetScene().SpawnLight(LightType::Point);
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ 5.0f, 5.0f, 0.0f });
        pLight->SetIntensity(100.0f);
        pLight->SetRange(10.0f);
        pLight->SetColor({ 1.0f, 1.0f, 1.0f });
    }
    */

    // Spot
    /*
    {
        lightHandle  = m_pEngine->GetScene().SpawnLight(LightType::Spot);
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ 0.0f, 5.0f, 0.0f });
        pLight->GetTransform().SetRotation(90.0f, 0.0f, 0.0f);
        pLight->SetIntensity(100.0f);
        pLight->SetRange(10.0f);
        pLight->SetColor({ 1.0f, 1.0f, 1.0f });
        pLight->SetSpotAngles(5.0f, 15.0f);
    }
    */

    // 2 point lights
    /*
    {
        lightHandle  = m_pEngine->GetScene().SpawnLight(LightType::Point);
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ 5.0f, 5.0f, 0.0f });
        pLight->SetIntensity(100.0f);
        pLight->SetRange(10.0f);
        pLight->SetColor({ 1.0f, 0.0f, 0.0f });

        lightHandle = m_pEngine->GetScene().SpawnLight(LightType::Point);
        pLight      = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ -5.0f, 5.0f, 0.0f });
        pLight->SetIntensity(100.0f);
        pLight->SetRange(10.0f);
        pLight->SetColor({ 0.0f, 0.0f, 1.0f });
    }
    */

    // 2 photometric lights
    {
        // IESプロファイルのロード
        std::optional<uint32_t> iesIndex;
        AssetPath().GetAssetPath(L"ies/TopPost.IES", path);
        iesIndex = loader.LoadIESProfile(path);
        assert(iesIndex.has_value() && "Failed to load IES profile.");

        lightHandle  = m_pEngine->GetScene().SpawnLight(LightType::Photometric);
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ 3.0f, 3.0f, 0.0f });
        pLight->GetTransform().SetRotation(90.0f, 0.0f, 0.0f);
        pLight->SetColor({ 1.0f, 1.0f, 1.0f });
        pLight->SetIntensity(10.0f);
        pLight->SetIESIndex(iesIndex.value());

        std::optional<uint32_t> iesIndex2;
        AssetPath().GetAssetPath(L"ies/Bollard.IES", path);
        iesIndex2 = loader.LoadIESProfile(path);
        assert(iesIndex2.has_value() && "Failed to load IES profile.");

        lightHandle = m_pEngine->GetScene().SpawnLight(LightType::Photometric);
        pLight      = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().SetPosition({ -3.0f, 3.0f, 0.0f });
        pLight->GetTransform().SetRotation(90.0f, 0.0f, 0.0f);
        pLight->SetColor({ 1.0f, 1.0f, 1.0f });
        pLight->SetIntensity(10.0f);
        pLight->SetIESIndex(iesIndex2.value());
    }
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
        if (!m_appleObject.IsValid()) {
            m_appleObject = m_pEngine->GetScene().SpawnObject(m_appleModel);
            m_pEngine->GetScene()
                .GetObject(m_appleObject)
                ->GetTransform()
                .SetScale({ 3.0f, 3.0f, 3.0f });
        } else {
            m_pEngine->GetScene().DespawnObject(m_appleObject);
            m_appleObject = {};
        }
    }

    if (m_pInputSystem->WasKeyPressed('3')) {
        if (!m_katanaObject.IsValid()) {
            m_katanaObject = m_pEngine->GetScene().SpawnObject(m_katanaModel);
        } else {
            m_pEngine->GetScene().DespawnObject(m_katanaObject);
            m_katanaObject = {};
        }
    }

    if (m_pInputSystem->WasKeyPressed('4')) {
        if (!m_planeObject.IsValid()) {
            m_planeObject = m_pEngine->GetScene().SpawnObject(m_planeModel);
            m_pEngine->GetScene()
                .GetObject(m_planeObject)
                ->GetTransform()
                .SetPosition({ 0.0f, -1.0f, 0.0f });
        } else {
            m_pEngine->GetScene().DespawnObject(m_planeObject);
            m_planeObject = {};
        }
    }
}