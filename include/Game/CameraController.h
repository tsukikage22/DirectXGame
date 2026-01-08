#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Engine/Camera.h"
#include "Engine/InputSystem.h"

class CameraController {
public:
    CameraController();
    ~CameraController();

    // 初期化
    void Init(Camera* pCamera, InputSystem* pInputSystem);

    // 終了処理
    void Term();

    // 更新
    void Update();

private:
    Camera* m_pCamera;
    InputSystem* m_pInputSystem;

    // パラメータ
    float m_moveSpeed;    // 移動速度
    float m_rotateSpeed;  // 回転速度
};