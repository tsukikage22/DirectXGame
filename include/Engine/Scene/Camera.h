/// @file Camera.h
/// @brief カメラの管理
#pragma once

#include <DirectXMath.h>

#include "Engine/Scene/Transform.h"

class Camera {
public:
    Camera();

    //================================
    // カメラの設定
    //================================
    /// @brief 垂直視野角の設定
    /// @param fovYDeg 垂直視野角（度）
    void SetFovYDeg(float fovYDeg) {
        m_fovYRad = DirectX::XMConvertToRadians(fovYDeg);
    };

    /// @brief アスペクト比の設定
    /// @param aspect
    void SetAspect(float aspect) { m_aspect = aspect; };

    /// @brief 最も近い描画距離の設定
    /// @param nearZ
    void SetNearZ(float nearZ) { m_nearZ = nearZ; };

    /// @brief 最も遠い描画距離の設定
    /// @param farZ
    void SetFarZ(float farZ) { m_farZ = farZ; };

    //================================
    // 行列の計算
    //================================
    DirectX::XMFLOAT4X4 GetViewMatrix();

    DirectX::XMFLOAT4X4 GetProjectionMatrix();

    //================================
    // アクセサ
    //================================
    Transform& GetTransform() { return m_transform; }
    const Transform& GetTransform() const { return m_transform; }

private:
    Transform m_transform;    // 位置や姿勢
    float m_fovYRad;          // 垂直視野角（ラジアン）
    float m_aspect;           // アスペクト比
    float m_nearZ = 1.0f;     // 描画範囲（最小）
    float m_farZ  = 1000.0f;  // 描画範囲（最大）

    // 行列
    DirectX::XMFLOAT4X4 m_viewMatrix;  // ビュー行列
    DirectX::XMFLOAT4X4 m_projMatrix;  // 射影行列
};
