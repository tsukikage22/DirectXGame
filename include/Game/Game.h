#pragma once

#include <memory>

// 前方宣言
class Engine;
class CameraController;

class Game {
public:
    Game();
    ~Game();

    // 初期化
    void Init(Engine* pEngine);

    // 終了処理
    void Term();

    // 更新
    void Tick();

private:
    Engine* m_pEngine;
    std::unique_ptr<CameraController> m_pCameraController;
};