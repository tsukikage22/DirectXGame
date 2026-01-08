/// @file   Application.h
/// @brief  メインループ本体
#pragma once

#include "App/Window.h"
#include "Engine/Engine.h"
#include "Game/Game.h"

class Application {
public:
    Application();
    ~Application();

    int Run();

private:
    //==================================
    // private members
    //==================================
    Window m_Window;           // ウィンドウ
    Engine m_Engine;           // エンジン
    Game m_Game;               // ゲームロジック
    bool m_isRunning = false;  // 実行中フラグ

    //==================================
    // private methods
    //==================================
    bool Init();  // 初期化
    void Term();  // 終了処理

    void MainLoop();  // メインループ
};