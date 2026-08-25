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
        &m_pEngine->GetScene().GetCamera(), &m_pEngine->GetInputSystem());

    m_pEngine->GetScene().GetCamera().SetExposure(2.8f, 1.0f / 30.0f, 800.0f);

    // HDRIの読み込みとキューブマップの構築
    std::filesystem::path path;
    if (!AssetPath().GetAssetPath(L"HDRI/venice_sunset_4k.hdr", path)) {
        OutputDebugStringW(L"Failed to find HDRI file.\n");
    }
    if (!m_pEngine->BuildEnvironmentMap(path)) {
        OutputDebugStringW(L"Failed to build environment map.\n");
    }

    // モデルのロード
    auto loader = m_pEngine->CreateAssetLoadScope();
    AssetPath().GetAssetPath(L"model/TextureSphere.glb", path);
    m_earthModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/lowpoly_apple.glb", path);
    m_appleModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/Katana.glb", path);
    m_katanaModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/Plane.glb", path);
    m_planeModel = loader.LoadModel(path);
    AssetPath().GetAssetPath(L"model/NormalTangentTest.glb", path);
    m_normalTestModel = loader.LoadModel(path);

    // ライトの作成

    // Directional
    {
        m_pEngine->GetScene().SpawnDirectionalLight({
            .direction   = { 0.0f, -1.0f, 0.0f },
            .color       = { 1.0f, 1.0f, 1.0f },
            .illuminance = 100000.0f,
        });
    }

    // 2 Spot lights
    /*
    {
        lightHandle  = m_pEngine->GetScene().SpawnSpotLight({
            .position = { 3.0f, 3.0f, 0.0f },
        });
        auto* pLight = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().LookAt({ 0.0f, 0.0f, 0.0f });

        lightHandle = m_pEngine->GetScene().SpawnSpotLight({
            .position = { -3.0f, 3.0f, 0.0f },
        });
        pLight      = m_pEngine->GetScene().GetLight(lightHandle);
        pLight->GetTransform().LookAt({ 0.0f, 0.0f, 0.0f });
    }
    */

    // 2 point lights
    /*
    {
        m_pEngine->GetScene().SpawnPointLight({
            .position     = { 3.0f, 3.0f, 0.0f },
            .color        = { 1.0f, 0.0f, 0.0f },
            .luminousFlux = 100000.0f,
            .range        = 20.0f,
        });

        m_pEngine->GetScene().SpawnPointLight({
            .position     = { -3.0f, 3.0f, 0.0f },
            .color        = { 0.0f, 0.0f, 1.0f },
            .luminousFlux = 100000.0f,
            .range        = 20.0f,
        });
    }
    */

    // 2 photometric lights
    /*
    {
        // IESプロファイルのロード
        std::optional<uint32_t> iesIndex;
        AssetPath().GetAssetPath(L"ies/TopPost.IES", path);
        iesIndex = loader.LoadIESProfile(path);
        assert(iesIndex.has_value() && "Failed to load IES profile.");

        m_pEngine->GetScene().SpawnPhotometricLight({
            .position     = { 3.0f, 3.0f, 0.0f },
            .direction    = { 0.0f, -1.0f, 0.0f },
            .luminousFlux = 9000.0f,
            .range        = 20.0f,
            .iesIndex     = iesIndex.value(),
        });

        std::optional<uint32_t> iesIndex2;
        AssetPath().GetAssetPath(L"ies/Bollard.IES", path);
        iesIndex2 = loader.LoadIESProfile(path);
        assert(iesIndex2.has_value() && "Failed to load IES profile.");

        m_pEngine->GetScene().SpawnPhotometricLight({
            .position     = { -3.0f, 3.0f, 0.0f },
            .direction    = { 0.0f, -1.0f, 0.0f },
            .luminousFlux = 9000.0f,
            .range        = 20.0f,
            .iesIndex     = iesIndex2.value(),
        });
    }
        */
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

    if (m_pInputSystem->WasKeyPressed('5')) {
        if (!m_normalTestObject.IsValid()) {
            m_normalTestObject =
                m_pEngine->GetScene().SpawnObject(m_normalTestModel);
        } else {
            m_pEngine->GetScene().DespawnObject(m_normalTestObject);
            m_normalTestObject = {};
        }
    }
}