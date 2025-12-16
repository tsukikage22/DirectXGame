#include "App/Application.h"

Application::Application()  = default;
Application::~Application() = default;

int Application::Run() {
    if (!Init()) {
        return -1;
    }

    MainLoop();

    Term();

    return 0;
}

// 初期化（ウィンドウの作成，D3D・ゲームロジックの初期化）
bool Application::Init() {
    // ウィンドウの作成
    const int windowWidth  = 1280;
    const int windowHeight = 720;

    if (!m_Window.Create(windowWidth, windowHeight, L"DirectX Game")) {
        return false;
    }

    // エンジンの初期化
    if (!m_Engine.Initialize()) {
        return false;
    }

    return true;
}

void Application::Term() {
    // ウィンドウの破棄
    m_Window.Destroy();

    // エンジンの終了処理
    m_Engine.Shutdown();
}

void Application::MainLoop() {
    // 実行中フラグを立てる
    m_isRunning = true;

    // メインループ
    while (m_isRunning) {
        // 1.メッセージポンプ
        // OSからのメッセージを処理する
        if (!m_Window.ProcessMessages()) {
            m_isRunning = false;
            break;
        }

        // 2. 描画処理
        // フレーム開始
        m_Engine.BeginFrame();

        // 定数バッファの書き込み
        m_Engine.Update();

        // 描画コマンド発行
        m_Engine.Render();

        // フレーム終了
        m_Engine.EndFrame();

        // 画面表示
        m_Engine.Present();
    }
}