#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Camera.h"

class CameraController
{
public:
    CameraController();
    ~CameraController();

    // 初期化
    void Init(Camera* pCamera, InputSystem* pInputSystem);

    // 終了処理
    void Term();

    // 更新
    void Update(float deltaTime);

    //----------------------------------------------
    // アクセサ
    //----------------------------------------------

    /// @brief ピッチ角を設定する
    /// @param pitch ピッチ角（度）
    void SetPitch(float pitch)
    {
        m_pitch = pitch;
    }

    /// @brief ヨー角を設定する
    /// @param yaw ヨー角（度）
    void SetYaw(float yaw)
    {
        m_yaw = yaw;
    }

private:
    // Forwardベクトルからオイラー角への変換（roll=0のためForwardから求められる）
    static DirectX::XMFLOAT2 ForwardToEuler(const DirectX::XMFLOAT3& forward);

    Camera* m_pCamera;
    InputSystem* m_pInputSystem;

    // パラメータ
    float m_moveSpeed;   // 移動速度
    float m_sensitivity; // マウス感度
    float m_pitch;       // ピッチ角（度）
    float m_yaw;         // ヨー角（度）
};
